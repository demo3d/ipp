#include <ipp/resource/resourcebuffer.hpp>
#include <ipp/resource/package.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::resource;

ResourceBuffer::ResourceBuffer(Package& package, string resourcePath)
    : _package{package}
    , _resourcePath{move(resourcePath)}
{
}

ResourceManager& ResourceBuffer::getResourceManager() const
{
    return _package.getResourceManager();
}
