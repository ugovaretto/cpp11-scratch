//
// Author: Ugo Varetto
//
// Reference custom allocator
//


#include <cstddef> //std::size_t
#include <memory>  //std::allocator
#include <stdexcept>
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//@WARNING never unmapped!
template < typename T >
class MMapAllocator {
public:
    //required
    using value_type = T;
    T* allocate(size_t n) {
        if(bounds_.second + n - bounds_.first > size_) {
           Remap(2 * Size() / sizeof(T));
        }
        bounds_.second += n;
        return bounds_.second;
    }
    void deallocate(T* p, size_t n) {
        if(n == 0) return;
        if(n > bounds_.second - bounds_.first) {
            throw std::logic_error(
                    "Deallocation size larger than allocated buffer");
        }
        bounds_.second -= n;
    }
    //MM specific
    void Sync() {
        const auto result = msync(bounds_.first,
                             Size(),
                             MS_SYNC | MS_INVALIDATE);
    }
    size_t Size() const { return sizeof(T) * (bounds_.second - bounds_.first); }
    void Close() {
        if (munmap(bounds_.first, Size()) == -1) {
            throw std::runtime_error("Error unmapping file");
        }
        close(fd_);
    }
    //optional
    MMapAllocator(const std::string& fpath, const size_t size = 0x10000)
            : fpath_(fpath), fd_(0), size_(size / sizeof(T)),
              bounds_(std::pair<T*, T*>(nullptr, nullptr))  {
        fd_ = open(fpath_.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
        if (fd_ == -1) {
            throw std::runtime_error("Cannot open file " + fpath_);
        }
        int result = lseek(fd_, size-1, SEEK_SET);
        if(result == -1) {
            close(fd_);
            throw std::runtime_error("Cannot seek on file " + fpath_);
        }
        result = write(fd_, "", 1);
        if (result != 1) {
            close(fd_);
            throw std::runtime_error("Cannot write to file " + fpath_);
        }
        bounds_.first = reinterpret_cast< T* >(
                    mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd_, 0));
        if(bounds_.first == nullptr) {
            close(fd_);
            throw std::runtime_error("Cannot map memory");
        }
        bounds_.second = bounds_.first;
    }
private:
    void Remap(size_t elements) {
        if (munmap(bounds_.first, Size()) == -1) {
            throw std::runtime_error("Error unmapping file");
        }
        const size_t n = Size() / sizeof(T);
        rewind(fd_);
        lseek(fd_, sizeof(T) * elements - 1, SEEK_SET);
        write(fd_, "", 1);
        bounds_.first = reinterpret_cast< T* >(
                mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fd_, 0));
        bounds_.second = bounds_.first + n;
    }
private:
    std::string fpath_;
    int fd_;
    size_t size_;
    using Bounds = std::pair< T*, T* >;
    Bounds bounds_;
};


using namespace std;

int main(int, char **) {

    //allocator
    MMapAllocator<int> a1("MMAP1");
    int *a = a1.allocate(10);

    a[0] = 1;
    a[2] = 3;
    a[9] = 7;

    //a1.Sync();

    //a1.Close();
    //cout << a[9] << endl;

    //a1.deallocate(a, 10);

    return 0;
}



//
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/mmap.h>
//
//#define FILEPATH "/tmp/mmapped.bin"
//#define NUMINTS  (1000)
//#define FILESIZE (NUMINTS * sizeof(int))
//
//int main(int argc, char *argv[])
//{
//    int i;
//    int fd;
//    int result;
//    int *map;  /* mmapped array of int's */
//
//    /* Open a file for writing.
//     *  - Creating the file if it doesn't exist.
//     *  - Truncating it to 0 size if it already exists. (not really needed)
//     *
//     * Note: "O_WRONLY" mode is not sufficient when mmaping.
//     */
//    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
//    if (fd == -1) {
//        perror("Error opening file for writing");
//        exit(EXIT_FAILURE);
//    }
//
//    /* Stretch the file size to the size of the (mmapped) array of ints
//     */
//    result = lseek(fd, FILESIZE-1, SEEK_SET);
//    if (result == -1) {
//        close(fd);
//        perror("Error calling lseek() to 'stretch' the file");
//        exit(EXIT_FAILURE);
//    }
//
//    /* Something needs to be written at the end of the file to
//     * have the file actually have the new size.
//     * Just writing an empty string at the current file position will do.
//     *
//     * Note:
//     *  - The current position in the file is at the end of the stretched
//     *    file due to the call to lseek().
//     *  - An empty string is actually a single '\0' character, so a zero-byte
//     *    will be written at the last byte of the file.
//     */
//    result = write(fd, "", 1);
//    if (result != 1) {
//        close(fd);
//        perror("Error writing last byte of the file");
//        exit(EXIT_FAILURE);
//    }
//
//    /* Now the file is ready to be mmapped.
//     */
//    map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//    if (map == MAP_FAILED) {
//        close(fd);
//        perror("Error mmapping the file");
//        exit(EXIT_FAILURE);
//    }
//
//    /* Now write int's to the file as if it were memory (an array of ints).
//     */
//    for (i = 1; i <=NUMINTS; ++i) {
//        map[i] = 2 * i;
//    }
//
//    /* Don't forget to free the mmapped memory
//     */
//    if (munmap(map, FILESIZE) == -1) {
//        perror("Error un-mmapping the file");
//        /* Decide here whether to close(fd) and exit() or not. Depends... */
//    }
//
//    /* Un-mmaping doesn't close the file, so we still need to do that.
//     */
//    close(fd);
//    return 0;
//}

