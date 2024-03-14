// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "db.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

std::string imvector_to_string(ImVector<char> vec) {
	std::string str;
	for (int i = 0; i < vec.size(); i++) {
		str += vec[i];
	}
	return str;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct Funcs
{
    static int MyResizeCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            ImVector<char>* my_str = (ImVector<char>*)data->UserData;
            IM_ASSERT(my_str->begin() == data->Buf);
            my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1

            data->Buf = my_str->begin();
        }
        return 0;
    }

    // Note: Because ImGui:: is a namespace you would typically add your own function into the namespace.
    // For example, you code may declare a function 'ImGui::InputText(const char* label, MyString* my_str)'
    static bool MyInputTextMultiline(const char* label, ImVector<char>* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        return ImGui::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
    }

    static bool MyInputTextWithHint(const char* label, const char* hint_text, ImVector<char>* my_str, ImGuiInputTextFlags flags = 0)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        return ImGui::InputTextWithHint(label, hint_text, my_str->begin(), (size_t)my_str->size(), flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
    }
};

// Main code
int main(int, char**)
{
    // Initialize the database
    DB db;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Start with the window maximized
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GuiNotes", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_login_window = true;
    static char username[64];
    static char password[64];
    static bool rename = false;
    static std::string title;
    static std::string content;
    static bool selected = false;
    static unsigned int note_id;
    static ImVector<char> title_str;
    static ImVector<char> content_str;
    ImVec4 clear_color = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main Menu
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Arquivos")) {
                if (ImGui::MenuItem("Novo", "Ctrl+N")) { /* Do stuff */ }
                if (ImGui::MenuItem("Salvar", "Ctrl+S")) { /* Do stuff */ }
                if (ImGui::MenuItem("Exportar", "Ctrl+E")) { /* Do stuff */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Sair")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Login"))
            {
                if (db.IsLoggedIn()) {
                    ImGui::SeparatorText(("Logado como %s", db.GetUsername()));
                    if (ImGui::MenuItem("Logout")) {
                        db.Logout();
                    }
                }
                else {
                    if (ImGui::MenuItem("Login")) {
                        show_login_window = true;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Basic App
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one or the other.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        if (!selected) {
            if (ImGui::Begin("Main", &use_work_area, flags))
            {
                std::vector<std::tuple<std::string, std::string>> notes = db.GetNotes();
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                unsigned int id = 0;
                for (std::vector<std::tuple<std::string, std::string>>::const_iterator i = notes.begin(); i != notes.end(); ++i) {
                    std::string _title = std::get<0>(*i);
                    std::string _content = std::get<1>(*i);
                    ImGui::BeginChild(("Child ##%d", id), ImVec2(0, 256), ImGuiChildFlags_Border, window_flags);

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        title = _title;
                        content = _content;
                        selected = true;
                        title_str.Data = (char*)title.c_str();
                        content_str.Data = (char*)content.c_str();
                    }

                    const int length = _content.length();
                    char* content_char = new char[length + 1];
                    strcpy(content_char, _content.c_str());

                    ImGui::Text(_title.c_str());
                    char buff[25];
                    snprintf(buff, sizeof(buff), "##%d", id);
                    ImGui::InputTextMultiline(buff, content_char, IM_ARRAYSIZE(content_char), ImVec2(0, 0), ImGuiInputTextFlags_ReadOnly);

                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                }
            }
            ImGui::End();
        }
        else {

            // Note Window
            if (ImGui::Begin("Note")) {

                if (rename) {
                    if (title_str.empty()) {
                        title_str.Data = (char*)title.c_str();
					}
                    if (Funcs::MyInputTextWithHint("", "Escreva um título", &title_str, ImGuiInputTextFlags_EnterReturnsTrue)) {
                        std::string new_title = imvector_to_string(title_str);
                        db.SaveNoteTitle(&title, &new_title);
                        rename = false;
                        std::vector<std::tuple<std::string, std::string>> notes = db.GetNotes();
                    };
                }
                else {
                    ImGui::Text(title.c_str());
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        rename = true;
                    }
                }

                // For this demo we are using ImVector as a string container.
                // Note that because we need to store a terminating zero character, our size/capacity are 1 more
                // than usually reported by a typical string class.
                if (content_str.empty())
                    content_str.Data = (char*)content.c_str();
                Funcs::MyInputTextMultiline("", &content_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
            } ImGui::End();
        }

        // Login Window
        if (show_login_window) {
            if (ImGui::Begin("Login")) {
                ImGui::InputText("Username", username, 64);
                ImGui::InputText("Password", password, 64, ImGuiInputTextFlags_Password);
                if (ImGui::Button("Login")) {
                    try
                    {
                        db.Login(username, password);
                        ImGui::OpenPopup("Login efetuado com sucesso.");
                        show_login_window = false;
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << e.what() << std::endl;
                        if (strcmp(e.what(), "InvalidUsernameException: Username does not exist") == 0) {
                            ImGui::OpenPopup("Usuário não encontrado.");
                        }
                        else if (strcmp(e.what(), "InvalidPasswordException: Password does not match") == 0) {
                            ImGui::OpenPopup("Senha incorreta.");
                        }
                        else {
                            ImGui::OpenPopup("Erro desconhecido.");
                        }
                    }
                }
            }ImGui::End();
        }

        // Popups
        if (ImGui::BeginPopupModal("Login efetuado com sucesso.", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Login efetuado com sucesso.");
			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
        if (ImGui::BeginPopupModal("Usuário não encontrado.", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Usuário não encontrado.");
			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

        if (ImGui::BeginPopupModal("Senha incorreta.", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Senha incorreta.");
            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Erro desconhecido.", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Erro desconhecido.");
			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

        // Logged in
        if(db.IsLoggedIn()) {
			if (ImGui::Begin("Logged in")) {
				ImGui::Text("Logged in as %s", db.GetUsername());
				if (ImGui::Button("Logout")) {
					db.Logout();
				}
			} ImGui::End();
		}

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}