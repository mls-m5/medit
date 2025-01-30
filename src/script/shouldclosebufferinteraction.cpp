#include "script/interaction.h"
#include "script/staticcommandregister.h"
#include "views/editor.h"

namespace {

void beginCloseBufferInteraction(std::shared_ptr<IEnvironment> env) {
    auto i = Interaction{
        .text = "",
        .title =
            "close buffer " + env->editor().file()->path().filename().string(),
    };
}

StaticCommandRegister shouldCloseBufferReg{"close_buffer",
                                           beginCloseBufferInteraction};

} // namespace
