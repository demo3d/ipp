#include <ipp/resource/packageipparchive.hpp>
#include <ipp/schema/resource/archive_generated.h>
#include <fstream>

using namespace std;
using namespace ipp;
using namespace ipp::resource;

PackageIPPArchive::PackageIPPArchive(ResourceManager& resourceManager,
                                     string name,
                                     string archivePath)
    : Package(resourceManager, name)
    , _archivePath{move(archivePath)}
{
    ifstream archive(_archivePath, ios::binary);
    if (!archive) {
        IVL_LOG_THROW_ERROR(runtime_error, "Unable to open archive file : {}", _archivePath);
    }
    uint32_t headerSize;
    archive.read(reinterpret_cast<char*>(&headerSize), sizeof(uint32_t));
    IVL_LOG(Trace, "Created archive {} with header size {}", _archivePath, headerSize);

    std::vector<char> headerBuffer(headerSize);
    archive.read(headerBuffer.data(), headerSize);
    auto archiveHeader = schema::resource::archive::GetArchive(headerBuffer.data());

    uint32_t fileOffset = sizeof(uint32_t) + headerSize;
    for (auto fileDefinition : *archiveHeader->files()) {
        auto fileSize = fileDefinition->size();
        IVL_LOG(Trace, "Read archive {} file {} offset {}, size {}", _archivePath,
                fileDefinition->name()->str(), fileOffset, fileSize);
        _files.emplace(fileDefinition->name()->str(), FileReference{fileSize, fileOffset});
        fileOffset += fileSize;
    }
}

unique_ptr<ResourceBuffer> PackageIPPArchive::requestResourceData(const string& resourcePath)
{
    auto resourcePackagePath = resourcePath.substr(resourcePath.find(':') + 1);
    auto fileIt = _files.find(resourcePackagePath);
    if (fileIt == _files.end()) {
        IVL_LOG_THROW_ERROR(runtime_error,
                            "Unable to find resource data definition for {} in archive {}",
                            resourcePackagePath, _archivePath);
    }
    auto fileDefinition = fileIt->second;
    vector<char> buffer(fileDefinition.size);
    {
        ifstream archive(_archivePath, ios::binary);
        if (!archive) {
            IVL_LOG_THROW_ERROR(runtime_error, "Unable to open archive file : {}", _archivePath);
        }
        archive.seekg(fileDefinition.offset);
        archive.read(buffer.data(), fileDefinition.size);
        if (!archive) {
            IVL_LOG_THROW_ERROR(
                runtime_error,
                "Unable to read archive file : {} from archive : {}, current offset : {}",
                resourcePackagePath, _archivePath, archive.tellg());
        }
    }
    return make_unique<ResourceBufferMemory>(*this, resourcePackagePath, buffer);
}
