#pragma once

#include "opencv2/opencv.hpp"

#include "imgui.h"
#include "gl/gl3w.h"

class ImageWindow
{
private:
	// バッファ
	unsigned char* imagebuf = nullptr;

	// 画像情報
	int width = 0;
	int height = 0;
	int channel = 0;

	// windowの名前
	std::string myname;

	// テクスチャ
	GLuint gltxtname = 0;
	ImTextureID imtxtname = 0;

	// ステータス
	bool sized = false;
	bool enable = true;

	bool focus = false;

	// マウスで拡大縮小するための記憶
	ImVec2 imagePos;

	float imageScale = 1.0f;

	bool fitted = true;

public:

	ImageWindow()
	{}

	// とりあえず
	ImageWindow(std::string& name) :
		myname(name)
	{}

	ImageWindow(const char* name) :
		myname(name)
	{}

	// VIにまかせる
	ImageWindow(std::string& name, int w, int h, int ch, unsigned char* data) {
		myname = name;
	
		CopyBuffer(w, h, ch, data);
	}

	void CopyBuffer(int w, int h, int ch, unsigned char* data) {
		width = w;
		height = h;
		channel = ch;

		// 呼び元のバッファをコピーする
		size_t size = (size_t)w * h * ch;
		imagebuf = (unsigned char*)malloc(size);
		memcpy_s(imagebuf, size, data, size);

		glGenTextures(1, &gltxtname);
	}

	// copy
	ImageWindow(const ImageWindow& moto) :
		gltxtname(moto.gltxtname),
		imtxtname(moto.imtxtname),
		myname(moto.myname),
		width(moto.width),
		height(moto.height),
		channel(moto.channel)
	{
		//深いコピー
		size_t size = (size_t)width * height * channel;
		imagebuf = (unsigned char*)malloc(size);
		memcpy_s(imagebuf, size, moto.imagebuf, size);
	}

	// move
	ImageWindow(ImageWindow &&moto) noexcept
	{
		*this = std::move(moto);
	}

	ImageWindow& operator=(ImageWindow&& moto) noexcept
	{
		if (this != &moto) {
			// moveの処理
			gltxtname = moto.gltxtname;
			imtxtname = moto.imtxtname;
			myname = moto.myname;

			width = moto.width;
			height = moto.height;
			channel = moto.channel;
			imagebuf = moto.imagebuf;
			moto.imagebuf = nullptr; // 元のbuffer を引き継ぎ、元は消す
		}
		return *this;
	}


	~ImageWindow() {
		// buffer を消す
		ClearBuffer();
	}

	void ClearBuffer() {
		enable = false;
		if (imagebuf != nullptr) {
			free(imagebuf);
			imagebuf = nullptr;
		}
	}

	void Display(bool fcs);

	bool Cashed()
	{
		return imagebuf != nullptr;
	}

	void Enable()
	{
		enable = true;
		focus = true;
	}
};

