#include <udaq/helpers/imgui.h>

namespace  udaq::helpers::imgui{

int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        std::string* str = (std::string*)data->UserData;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }

    return 0;
}

bool InputText(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputText(label, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, (void*)&str);
}

bool InputUInt32(const char *label, uint32_t& v, ImGuiInputTextFlags flags)
{
    return ImGui::InputScalar(label, ImGuiDataType_U32, (void*)v,  NULL, NULL, "%u", flags);
}

}
