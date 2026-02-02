#include "EngineFramework/Application.h"
#include "AppLayer.h"
#include "UILayer.h"

int main() 
{
    AlphaEngine::ApplicationSpecification appSpec;
    appSpec.Name = "Last Roll";
    appSpec.windowSpec.Width = 1920;
    appSpec.windowSpec.Height = 1080;

    AlphaEngine::Application application(appSpec);
    application.PushLayer<AlphaEngine::AppLayer>();
    application.PushLayer<AlphaEngine::UILayer>();
    application.Run();
    
}