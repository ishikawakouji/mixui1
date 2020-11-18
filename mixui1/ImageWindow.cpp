#include "ImageWindow.h"

void imguiFitCenter(const ImVec2& regionmax, const ImVec2& padding, const float frameh, const int cols, const int rows, float& scale, ImVec2& pos)
{
	// 拡大縮小
	float ws = (regionmax.x - padding.x * 2.f) / (float)cols;
	float hs = (regionmax.y - frameh - padding.y * 4.f) / (float)rows;
	scale = ws > hs ? hs : ws;

	// センタリング
	pos = ImVec2((regionmax.x + padding.x - cols * scale) * 0.5f, (regionmax.y + frameh - rows * scale) * 0.5f);
	ImGui::SetCursorPos(pos);

}

// BGRA をそのまま BGRA で表示
ImTextureID cvMatBGRA2gltexture(GLuint& texture, int cols, int rows, uchar* data)
{
    //GLuint texture;
    //glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //ImTextureID imtexture{ (ImTextureID)(intptr_t)texture };

    return (ImTextureID)(intptr_t)texture;
}


// BGR 3ch を BGRA として表示
ImTextureID cvMatBGR2BGRAgltexture(GLuint& texture, int cols, int rows, uchar* data)
{
    try {
        //GLuint texture;
        //glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cols, rows, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        //ImTextureID imtexture{ (ImTextureID)(intptr_t)texture };
    }
    catch (...) {
        std::cout << "tex err" << std::endl;
    }

    return (ImTextureID)(intptr_t)texture;
}

#if 0
ImTextureID cvMatBGR2gltexture(GLuint& texture, cv::Mat& image)
{
    //GLuint texture;
    //glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
    //ImTextureID imtexture{ (ImTextureID)(intptr_t)texture };

    return (ImTextureID)(intptr_t)texture;
}

ImTextureID cvMatRGB2gltexture(GLuint& texture, cv::Mat& image)
{
    //GLuint texture;
    //glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
    //ImTextureID imtexture{ (ImTextureID)(intptr_t)texture };

    return (ImTextureID)(intptr_t)texture;
}

#endif

//ImTextureID cvMatGray2gltexture(GLuint& texture, cv::Mat& image)
ImTextureID cvMatGray2gltexture(GLuint& texture, int cols, int rows, uchar* data)
{
    //GLuint texture;
    //glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, cols, rows, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
    //ImTextureID imtexture{ (ImTextureID)(intptr_t)texture };

    return (ImTextureID)(intptr_t)texture;
}



void ImageWindow::Display(bool fcs)
{
    if (!Cashed()) return;
    if (!enable) return;

    focus = fcs;

    int cols = width;
    int rows = height;
    uchar* data = imagebuf;

    // グローバル
    extern int glfwWidth;
    extern int glfwHeight;

    if (!sized) {
        ImGui::SetNextWindowSize(ImVec2(glfwWidth * 0.7f, glfwHeight * 0.7f));
        sized = true;
    }
    if (focus) {
        ImGui::SetNextWindowFocus();
    }
	ImGui::Begin(myname.c_str(), &enable, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

    ImGuiIO io = ImGui::GetIO();

    if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Middle) || io.MouseWheel != 0.0f )
    {
        fitted = false;
    }
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
    {
        fitted = true;
    }

    if (fitted) {
        imguiFitCenter(ImGui::GetContentRegionMax(), ImGui::GetStyle().FramePadding, ImGui::GetFrameHeight(), cols, rows, imageScale, imagePos);
    }
    else {
        // check wheel
        if (io.MouseWheel != 0.0f) {
            float nextscale = (1.0f + io.MouseWheel * 0.1f);
            
            if (nextscale < 0.05f) nextscale = 0.05f;
            if (imageScale * nextscale > 0.01f) {

                // 画面の中央
                float wx = (ImGui::GetContentRegionMax().x + ImGui::GetStyle().FramePadding.x) * 0.5f;
                float wy = (ImGui::GetContentRegionMax().y + ImGui::GetFrameHeight()) * 0.5f;

                // 中央と画像始点の差
                float dx = imagePos.x - wx;
                float dy = imagePos.y - wy;

                // スケールで調整
                dx *= nextscale;
                dy *= nextscale;

                // 新しい位置
                imagePos.x = wx + dx;
                imagePos.y = wy + dy;

                imageScale *= nextscale;
            }
        }
        else {
            ImVec2 deltapos = ImGui::GetMouseDragDelta(ImGuiMouseButton_::ImGuiMouseButton_Middle);

            imagePos.x += deltapos.x;
            imagePos.y += deltapos.y;
        }

        ImGui::SetCursorPos(imagePos);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_::ImGuiMouseButton_Middle);
    }

    ImTextureID imtexture = (ImTextureID)0;

    switch (channel) {
    case 4:
        imtexture = cvMatBGRA2gltexture(gltxtname, cols, rows, data);
        break;
    case 3:
        imtexture = cvMatBGR2BGRAgltexture(gltxtname, cols, rows, data);
        break;
    case 1:
        imtexture = cvMatGray2gltexture(gltxtname, cols, rows, data);
        break;
    }
        
    ImGui::Image(imtexture, ImVec2(cols * imageScale, rows * imageScale));

	ImGui::End();

    focus = false;
}
