#include <ipp/context.hpp>

using namespace std;
using namespace ipp;

Context::Context(json configuration)
    : _configuration{configuration}
    , _resourceManager(*this)
{
}

unique_ptr<scene::Scene> Context::createScene(const string& resourcePath)
{
    auto data = _resourceManager.requestResourceData(resourcePath);
    return make_unique<scene::Scene>(*this, move(data));
}
