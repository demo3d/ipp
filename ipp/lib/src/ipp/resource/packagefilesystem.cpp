#include <ipp/log.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <fstream>

using namespace std;
using namespace ipp::resource;

PackageFileSystem::PackageFileSystem(ResourceManager& resourceManager,
                                     std::string name,
                                     std::string rootPath)
    : Package(resourceManager, name)
    , _rootPath{std::move(rootPath)}
{
}

unique_ptr<ResourceBuffer> PackageFileSystem::requestResourceData(const string& resourcePath)
{
    auto resourcePackagePath = resourcePath.substr(resourcePath.find(':') + 1);
    auto filePath = _rootPath + "/" + resourcePackagePath;
    IVL_LOG(Info, "Creating FileSystem resource buffer for resource : {} with file path : {}",
            resourcePath, filePath);

    ifstream file(filePath, ios::binary);
    if (!file) {
        IVL_LOG_THROW_ERROR(runtime_error, "Unable to open resource file : {} with path : {}",
                            resourcePath, filePath);
    }

    vector<char> buffer;
    file.seekg(0, ios::end);
    buffer.resize(static_cast<size_t>(file.tellg()));
    file.seekg(0, ios::beg);
    file.read(buffer.data(), static_cast<fstream::off_type>(buffer.size()));
    IVL_LOG(Info, "Source buffer : {} ({}) size : {}", resourcePath, filePath, buffer.size());

    return make_unique<ResourceBufferMemory>(*this, filePath, move(buffer));
}
