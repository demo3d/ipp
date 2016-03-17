#include <ipp/resource/stringresource.hpp>

using namespace std;
using namespace ipp::resource;

template <>
const string SharedResourceT<StringResource>::ResourceTypeName = "StringResource";

StringResource::StringResource(unique_ptr<ResourceBuffer> data)
    : SharedResourceT<StringResource>(data->getResourceManager(), data->getResourcePath())
    , _data{data->getData(), data->getSize()}
{
}
