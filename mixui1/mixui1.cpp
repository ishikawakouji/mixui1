// mixui1.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "nfd.h"
#include <filesystem>

#include "utf8sjis.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include "ImageWindow.h"

/*
* 合成処理
* undef max の都合で本体は最後へ
*/
extern void mixImage(int& imageViewHeight, int& imageViewWidth, int numberOffset, int numberMix, int numberPitch, std::vector<cv::Mat>& imageBuffer, cv::Mat& imageRes);

// グローバル
int glfwWidth = 640;
int glfwHeight = 480;

int main()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(glfwWidth, glfwHeight, "Light Dear ImGui", NULL, NULL);

    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Window MAX
    glfwMaximizeWindow(window);

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 幅ないし高さに制限があるらしい
    GLint glMaxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxSize);


    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\meiryo.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\meiryo.ttc", 27.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    IM_ASSERT(font != NULL);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    // NFD用
    nfdchar_t* outFolder = nullptr;
    nfdresult_t nfdResult;
    nfdpathset_t outfiles;
    int numberReadOffset = 0;
    int numberReadMax = 80;

    // 合成処理
    bool flagMixOpe = false;
    int numberOffset = 0;
    int preNumberOffset = 0;
    int numberMix = 80;
    int preNumberMix = 1;
    int numberPitch = 300;
    int preNumberPitch = 0;
    std::vector<cv::Mat> imageBuffer;
    bool flagRedraw = false;

    // image window
    bool flagImageViewEnable = false;
    bool flagImageViewFocus = true;
    ImageWindow imageView;
    int imageViewWidth = 0;
    int imageViewHeight = 0;
    int imageViewChannels = 0;

    // 頭文字
    char atama[20] = { "RGB" };

    // 接続するファイルの確認
    int rgb = 0;
    bool BayerRG = true;
    int outch = CV_8UC3;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 元のウィンドウサイズを拾う
        glfwGetWindowSize(window, &glfwWidth, &glfwHeight);

        /*
        * フォルダを選択
        */
        ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));

        ImGui::Begin("Image Folder");
        ImGui::Text(u8"連続画像の合成");

        // 頭文字
        ImGui::InputText("head", atama, 20);

        // フォルダ、指定済みか
        static bool pickedFolder = false;

        // フォルダパス
        static std::string folderPath;

        // 全file名
        static std::vector<std::string> fileNames;

        if (ImGui::Button("Set Folder")) {
            // 頭文字チェック
            std::string namehead(atama);

            switch (namehead.at(0)) {
            case '2':
                rgb = 0;
                BayerRG = true;
                outch = CV_8UC3;
                break;
            case 'H':
            case 'M':
                rgb = 0;
                BayerRG = false;
                outch = CV_8UC1;
                break;
            case 'K':
                if (namehead == "Kizu") {
                    rgb = 0;
                    outch = CV_8UC1;
                }
                else {
                    rgb = 1;
                    outch = CV_8UC3;
                }
                BayerRG = false;
                break;
            default:
                // RGB
                rgb = 1;
                BayerRG = false;
                outch = CV_8UC3;
            }


            // NFD呼び出し
            nfdResult = NFD_PickFolder(nullptr, &outFolder);

            if (nfdResult == NFD_OKAY) {

                // 文字コード変換
                std::string u8val(outFolder);
                folderPath = utf8_to_wide_to_multi_winapi(u8val);
                free(outFolder);

                // 全file名を取得
                using namespace std::filesystem;
                directory_iterator iter(folderPath), end;
                std::error_code err;

                fileNames.clear();

                for (; iter != end; iter.increment(err)) {
                    if (iter->is_regular_file()) {
                        if (iter->path().extension().string() != ".db") {
                            if (iter->path().filename().string().substr(0, namehead.length()) == namehead) {
                                fileNames.push_back(iter->path().string());
                            }
                        }
                        else {
                            std::cout << iter->path().string() << " is not image file" << std::endl;
                        }
                    }
                }

                if (err) {
                    std::cout << err.value() << std::endl;
                    std::cout << err.message() << std::endl;
                    return false;
                }

                // file名をソート
                std::sort(fileNames.begin(), fileNames.end());

                /*
                // file 読み込み
                imageBuffer.clear();
                for (const auto file :fileNames) {
                    cv::Mat img = cv::imread(file, rgb);

                    if (img.empty()) { continue; };

                    // 色変換してみる
                    cv::Mat img2 = cv::Mat(img.rows, img.cols, outch);
                    if (BayerRG) {
                        //cv::cvtColor(img, img2, CV_BayerRG2BGR);
                        cv::cvtColor(img, img2, CV_BayerBG2BGR); // ライトの色を見て調整
                    }
                    else {
                        img2 = img;
                    }

                    //cv::Mat img3 = cv::Mat(img2.rows, img2.cols, CV_8UC4);
                    //cv::cvtColor(img2, img3, CV_BGR2BGRA);

                    imageBuffer.push_back(img2);
                }

                flagMixOpe = true;
                flagRedraw = true;
                */

            }
            else if (nfdResult == NFD_CANCEL) {
                puts("User pressed cancel.");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }
        }

        // 見つかったファイル数
        ImGui::Text("%d files found", fileNames.size());

        // 読み込むファイルの最初と最大個数を設定
        ImGui::InputInt("start", &numberReadOffset);
        ImGui::InputInt("MAX files", &numberReadMax);

        if (ImGui::Button("read files")) {
            pickedFolder = true;

            // file 読み込み
            imageBuffer.clear();
            //for (const auto file : fileNames) {
            for (int i=numberReadOffset; i < fileNames.size() && i-numberReadOffset < numberReadMax; ++i) {
                cv::Mat img = cv::imread(fileNames[i], rgb);

                if (img.empty()) { continue; };

                // 色変換してみる
                cv::Mat img2 = cv::Mat(img.rows, img.cols, outch);
                if (BayerRG) {
                    //cv::cvtColor(img, img2, CV_BayerRG2BGR);
                    cv::cvtColor(img, img2, CV_BayerBG2BGR); // ライトの色を見て調整
                }
                else {
                    img2 = img;
                }

                //cv::Mat img3 = cv::Mat(img2.rows, img2.cols, CV_8UC4);
                //cv::cvtColor(img2, img3, CV_BGR2BGRA);

                imageBuffer.push_back(img2);
            }

            // ファイル個数を反映
            numberMix = imageBuffer.size();

            flagMixOpe = true;
            flagRedraw = true;
        }

        if (pickedFolder) {
            ImGui::Text("%s", folderPath.c_str());
            if (ImGui::Button("image view")) {
                imageView.Enable();
            }
        }



        bool flagImm = false;

        //ImGui::Checkbox("imm.", &flagImm);

        flagImm |= ImGui::InputInt("offset", &preNumberOffset, 8, 8, ImGuiInputTextFlags_EnterReturnsTrue);

        flagImm |= ImGui::InputInt("mix num", &preNumberMix, 1, 1, ImGuiInputTextFlags_EnterReturnsTrue);

        if (flagImm) {

            if (numberOffset != preNumberOffset || numberMix != preNumberMix) {

                if (preNumberOffset < 0) {
                    preNumberOffset = 0;
                }

                if ((size_t)preNumberOffset + preNumberMix > imageBuffer.size()) {
                    preNumberOffset = (int)imageBuffer.size() - preNumberMix;
                    if (preNumberOffset < 0) {
                        preNumberOffset = 0;
                        preNumberMix = (int)imageBuffer.size();
                    }
                }
                if (preNumberMix < 1) {
                    preNumberMix = 1;
                }
                if (preNumberMix > imageBuffer.size()) {
                    preNumberMix = imageBuffer.size();
                }

                if (numberOffset != preNumberOffset || numberMix != preNumberMix) {
                    numberOffset = preNumberOffset;
                    numberMix = preNumberMix;

                    flagRedraw = true;
                }
            }
        }

        ImGui::SameLine();
        ImGui::Text("/ %d", imageBuffer.size());
        /*
        ImGui::SameLine();
        if (ImGui::Button(" set, redraw")) {
            flagRedraw;
        }
        */

        if (ImGui::InputInt("px interval", &numberPitch, 1, 2, ImGuiInputTextFlags_EnterReturnsTrue)) {
            /*
            if (numberPitch % 2 != 0) {
                numberPitch -= numberPitch % 2;
            }
            */
            if (numberPitch < 1) {
                numberPitch = 1;
            }
            if (preNumberPitch != numberPitch) {
                flagRedraw = true;
            }
        }

        if (flagMixOpe && flagRedraw) {
            // 合成
            cv::Mat imageRes;
            mixImage(imageViewHeight, imageViewWidth, numberOffset, numberMix, numberPitch, imageBuffer, imageRes);

            // 検算
            //bool hoge = imageRes.isContinuous();

            // GLのtextureのサイズに限界があるらしいので、これに合わせて先に縮小しておく
            if (imageViewWidth > glMaxSize) {
                float scaleImage = (float)glMaxSize / (float)imageViewWidth;
                int nextw = glMaxSize;
                int nexth = (int)(scaleImage * imageViewHeight);

                cv::Mat img1 = imageRes.clone();

                imageRes.create(nexth, nextw, img1.type());
                cv::resize(img1, imageRes, imageRes.size());

                imageViewWidth = nextw;
                imageViewHeight = nexth;
            }


            // 画像ビューア
            std::string viewname = "images";
            imageView.ClearBuffer();
            imageView = ImageWindow(viewname, imageViewWidth, imageViewHeight, imageRes.channels(), imageRes.data);
            imageView.Enable();

            preNumberMix = numberMix;
            preNumberPitch = numberPitch;

            flagRedraw = false;
        }

        // 保存
        if (ImGui::Button("save")) {
            int w;
            int h;
            cv::Mat imageRes;

            // 合成
            mixImage(h, w, numberOffset, numberMix, numberPitch, imageBuffer, imageRes);

            nfdchar_t* outPath;
            nfdResult = NFD_SaveDialog("png", NULL, &outPath);

            if (nfdResult == NFD_OKAY) {
                // 文字コード変換
                std::string u8val(outPath);
                std::string imagePath = utf8_to_wide_to_multi_winapi(u8val);
                free(outPath);

                // 保存
                cv::imwrite(imagePath, imageRes);
            }
            else if (nfdResult == NFD_CANCEL) {
                puts("User pressed cancel.");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }

        }

        
        ImGui::Separator();

        // 集合画像
        ImGui::Text(u8"同じ場所での合成");
        if (ImGui::Button("pick images")) {
            // NFD呼び出し
            nfdResult = NFD_OpenDialogMultiple(nullptr, nullptr, &outfiles);

            if (nfdResult == NFD_OKAY) {

                fileNames.clear();

                for (size_t i = 0; i < NFD_PathSet_GetCount(&outfiles); ++i) {
                    nfdchar_t* onefile = NFD_PathSet_GetPath(&outfiles, i);

                    // 文字コード変換
                    std::string u8val(onefile);
                    std::string onepath = utf8_to_wide_to_multi_winapi(u8val);
                    
                    fileNames.push_back(onepath);
                }
                NFD_PathSet_Free(&outfiles);

                // 集合処理
                int allImageHeight, allImageWidth;

                cv::Mat imageRes = cv::imread(fileNames[0]);

                allImageHeight = imageRes.rows;
                allImageWidth = imageRes.cols;

                for (int i = 1; i < fileNames.size(); ++i) {
                    cv::Mat img = cv::imread(fileNames[i]);

                    int oneh = img.rows;
                    int onew = img.cols;

                    // 場合分けが雑、まだ不足しているはず

                    if (allImageHeight > oneh || allImageWidth > onew) {
                        imageRes = imageRes(cv::Rect(0, 0, onew, oneh));
                        allImageHeight = oneh;
                        allImageWidth = onew;
                    }

                    if (allImageHeight < oneh || allImageWidth < onew) {
                        img = img(cv::Rect(0, 0, allImageWidth, allImageHeight));
                    }

                    cv::max(imageRes, img, imageRes);
                    
                }

                // GLのtextureのサイズに限界があるらしいので、これに合わせて先に縮小しておく
                if (allImageWidth > glMaxSize) {
                    float scaleImage = (float)glMaxSize / (float)allImageWidth;
                    int nextw = glMaxSize;
                    int nexth = (int)(scaleImage * allImageHeight);

                    cv::Mat img1 = imageRes.clone();

                    imageRes.create(nexth, nextw, img1.type());
                    cv::resize(img1, imageRes, imageRes.size());

                    allImageWidth = nextw;
                    allImageHeight = nexth;
                }

                // 画像ビューア
                std::string viewname = "images";
                imageView.ClearBuffer();
                imageView = ImageWindow(viewname, allImageWidth, allImageHeight, imageRes.channels(), imageRes.data);
                imageView.Enable();


            }
            else if (nfdResult == NFD_CANCEL) {
                puts("User pressed cancel.");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }
        }

        // 集合写真の保存
        if (ImGui::Button("composed image save")) {
            nfdchar_t* outPath;
            nfdResult = NFD_SaveDialog("png", NULL, &outPath);


            if (nfdResult == NFD_OKAY) {
                // 文字コード変換
                std::string u8val(outPath);
                std::string imagePath = utf8_to_wide_to_multi_winapi(u8val);
                free(outPath);

                // 集合処理
                int allImageHeight, allImageWidth;

                cv::Mat imageRes = cv::imread(fileNames[0]);

                allImageHeight = imageRes.rows;
                allImageWidth = imageRes.cols;

                for (int i = 1; i < fileNames.size(); ++i) {
                    cv::Mat img = cv::imread(fileNames[i]);

                    int oneh = img.rows;
                    int onew = img.cols;

                    // 場合分けが雑、まだ不足しているはず

                    if (allImageHeight > oneh || allImageWidth > onew) {
                        imageRes = imageRes(cv::Rect(0, 0, onew, oneh));
                        allImageHeight = oneh;
                        allImageWidth = onew;
                    }

                    if (allImageHeight < oneh || allImageWidth < onew) {
                        img = img(cv::Rect(0, 0, allImageWidth, allImageHeight));
                    }

                    cv::max(imageRes, img, imageRes);

                }

                // 保存
                cv::imwrite(imagePath, imageRes);
            }
            else if (nfdResult == NFD_CANCEL) {
                puts("User pressed cancel.");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }
        }

        ImGui::End();

        imageView.Display(false);


        // Rendering
        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();




}

/*
* 合成処理
* undef max の都合で本体は最後へ
* imageViewHeight, imageViewWidth, imageRes は出力
*/
void mixImage(int& imageViewHeight, int& imageViewWidth, int numberOffset, int numberMix, int numberPitch, std::vector<cv::Mat>& imageBuffer, cv::Mat& imageRes)
{
    // 合成後のサイズを計算
    imageViewHeight = imageBuffer[0].rows;
    imageViewWidth = imageBuffer[0].cols + numberPitch * (numberMix - 1);

    // 調整、4の倍数がいいらしい
    imageViewWidth = imageViewWidth + (imageViewWidth % 4 == 0 ? 0 : 4 - imageViewWidth % 4);

    // 検算
    //std::cout << imageViewWidth << std::endl;

    // 合成
    imageRes = cv::Mat::zeros(imageViewHeight, imageViewWidth, imageBuffer[0].type());
    int oneh = imageViewHeight;
    int onew = imageBuffer[0].cols;
    if (numberMix > imageBuffer.size()) { numberMix = (int)imageBuffer.size(); };

    for (int i = 0; i < numberMix; ++i) {
        cv::Mat roi = imageRes(cv::Rect(numberPitch * i, 0, onew, oneh));
        //imageBuffer[i].copyTo(roi);
        cv::Mat moi = cv::Mat(roi);
#undef max
        //moi = 
        ((cv::Mat)cv::max(roi, imageBuffer[numberOffset + i])).copyTo(roi);
        //moi.copyTo(roi);
    }
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
