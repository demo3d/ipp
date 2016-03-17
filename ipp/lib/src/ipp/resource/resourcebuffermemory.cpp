#include <ipp/resource/resourcebuffermemory.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::resource;

ResourceBufferMemory::ResourceBufferMemory(Package& package,
                                           string resourcePath,
                                           vector<char> buffer)
    : ResourceBuffer(package, move(resourcePath))
    , _buffer{move(buffer)}
{
}

const char* ResourceBufferMemory::getData(size_t offset, size_t length)
{
    assert(offset + length <= _buffer.size());
    return _buffer.data();
}

size_t ResourceBufferMemory::getSize() const
{
    return _buffer.size();
}
