#pragma once

#include "opencv2/opencv.hpp"

#include "imgui.h"
#include "gl/gl3w.h"

class ImageWindow
{
private:
	// �o�b�t�@
	unsigned char* imagebuf = nullptr;

	// �摜���
	int width = 0;
	int height = 0;
	int channel = 0;

	// window�̖��O
	std::string myname;

	// �e�N�X�`��
	GLuint gltxtname = 0;
	ImTextureID imtxtname = 0;

	// �X�e�[�^�X
	bool sized = false;
	bool enable = true;

	bool focus = false;

	// �}�E�X�Ŋg��k�����邽�߂̋L��
	ImVec2 imagePos;

	float imageScale = 1.0f;

	bool fitted = true;

public:

	ImageWindow()
	{}

	// �Ƃ肠����
	ImageWindow(std::string& name) :
		myname(name)
	{}

	ImageWindow(const char* name) :
		myname(name)
	{}

	// VI�ɂ܂�����
	ImageWindow(std::string& name, int w, int h, int ch, unsigned char* data) {
		myname = name;
	
		CopyBuffer(w, h, ch, data);
	}

	void CopyBuffer(int w, int h, int ch, unsigned char* data) {
		width = w;
		height = h;
		channel = ch;

		// �Ăь��̃o�b�t�@���R�s�[����
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
		//�[���R�s�[
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
			// move�̏���
			gltxtname = moto.gltxtname;
			imtxtname = moto.imtxtname;
			myname = moto.myname;

			width = moto.width;
			height = moto.height;
			channel = moto.channel;
			imagebuf = moto.imagebuf;
			moto.imagebuf = nullptr; // ����buffer �������p���A���͏���
		}
		return *this;
	}


	~ImageWindow() {
		// buffer ������
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

