#pragma once

#include <fstream>
#include <array>
#include <vector>

#include "utilDefs.h"
#include "PixelDefs.h"

namespace voi {
	

	class Image {
		Pixel* _data = nullptr;
		int _width = 0;
		int _height = 0;
		bool _alpha = false;

	public:
		Image() {}

		Image(int w, int h, bool a = true) {
			_width = abs(w);
			_height = abs(h);
			_data = new Pixel[_width * _height];
			_alpha = a;
		}

		Image(const Image& other) {
			if (other._data != nullptr) return;
			_width = other._width;
			_height = other._height;
			_alpha = other._alpha;
			_data = new Pixel[_width * _height];
			for (int i = 0; i < _width * _height; i++) {
				_data[i] = other._data[i];
			}
		}
		Image(Image&& other) {
			_width = other._width; other._width = 0;
			_height = other._height; other._height = 0;
			_data = other._data; other._data = nullptr;
			_alpha = other._alpha; other._alpha = false;
		}

		void operator = (const Image& other) {
			if (other._data != nullptr) return;
			_width = other._width;
			_height = other._height;
			_alpha = other._alpha;
			_data = new Pixel[_width * _height];
			for (int i = 0; i < _width * _height; i++) {
				_data[i] = other._data[i];
			}
		}
		void operator = (Image&& other) {
			_width = other._width; other._width = 0;
			_height = other._height; other._height = 0;
			_data = other._data; other._data = nullptr;
			_alpha = other._alpha; other._alpha = false;
		}

		~Image() {
			if (_data != nullptr) {
				delete[] _data;
				_data = nullptr;
			}
		}

		int width() const { return _width; }
		int height() const { return _height; }
		bool alpha() const { return _alpha; }
		const Pixel* data() const { return _data; }

		bool setPixel(int x, int y, Pixel color) {
			if (x >= 0 && x < _width && y >= 0 && y < _height) {
				_data[y * _width + x] = color;
				return true;
			}
			return false;
		}

		static Image ReadDecodeImage(const char* path) {
			Image img;

			std::ifstream file(path, std::ios::binary);
			if (file) {
				std::string extension(path);
				extension = extension.substr(extension.size() - 3, 3);

				if (!extension.compare("bmp")) {
					DecodeBMP(file, img);
				}
			}

			return std::move(img);
		}

	private:

		static void DecodeBMP(std::ifstream& file, Image& img) {

			enum compresion : ui32 {
				RGB, RLE8, RLE4, BITFIELDS, VOI_BM_JPEG, VOI_BM_PNG
			};

			union {
				struct BITMAP_INFO_HEADER {
					ui32 biHeaderSize;
					i32  biWidth;
					i32  biHeight;
					i16  biPlanes;
					i16  biBitCount;
					ui32 biCompression;
					ui32 biByteSize;
					i32  biXPPM;
					i32  biYPPM;
					ui32 biClrUsed;
					ui32 biClrImportant;
				} BI;

				struct BITMAP_V5_HEADER {
					ui32        headerSize;
					i32         width;
					i32         height;
					ui16        planes;
					ui16        bitCount;
					ui32        compression;
					ui32        byteSize;
					i32         xPPM;
					i32         yPPM;
					ui32        clrUsed;
					ui32        clrImportant;
					ui32        redMask;
					ui32        greenMask;
					ui32        blueMask;
					ui32        alphaMask;
					ui32        CSType;

					struct {

						i32 xRed;
						i32 yRed;
						i32 zRed;
						//------------------------------
						i32 xGreen;
						i32 yGreen;
						i32 zGreen;
						//------------------------------
						i32 xBlue;
						i32 yBlue;
						i32 zBlue;

					} Endpoints;

					ui32        gammaRed;
					ui32        gammaGreen;
					ui32        gammaBlue;
					ui32        intent;
					ui32        profileData;
					ui32        profileSize;
					ui32        reserved;
				} V5;
			};

			file.seekg(0, file.end);
			i32 fileSize = file.tellg();
			file.seekg(0, file.beg);

			if (fileSize < 14) return;

			char buff[256] = { 0 };
			file.read(buff, 14);

			std::string aux{ buff };

			if (aux.length() >= 2 && !aux.substr(0, 2).compare("BM")) {
				ui32 size = *((ui32*)&buff[2]);

				if (fileSize < size) return;

				ui32 offset = *((ui32*)&buff[10]);

				if (offset >= size) return;

				file.read(buff, 4);
				ui32 headerSize = *((ui32*)buff);

				if ((!headerSize) || (headerSize + 14 > offset)) return;

				file.read(buff + 4, (long long)headerSize - 4);

				if (headerSize == 124) {
					memcpy((void*)&V5, (void*)buff, sizeof(V5));

					//------------------------------------------
					// aun falta para que esto salga bien
					//------------------------------------------

					switch (V5.compression) {
					case BITFIELDS:
					{
						if (V5.bitCount == 32) {

							img._data = (Pixel*)(new ui8[V5.byteSize]);

							file.read((char*)img._data, V5.byteSize);

							img._alpha = true;
							img._width = V5.width;
							img._height = V5.height;
						}
					}
					break;
					default:
						return;
					}
				}
			}

			return;
		}
	};

}




















































////TODO: do properly
//class Image {
//public:
//	ui8 error = 0;
//	enum headerType : ui8 {
//		VOI_BM_V5_HEADER, VOI_BM_INF_HEADER
//	};
//	enum compresion : ui32 {
//		RGB, RLE8, RLE4, BITFIELDS, VOI_BM_JPEG, VOI_BM_PNG
//	};
//	enum ImgError : ui8 {
//		E_FILE = 0x01, E_ENCODING = 0x02, E_IMPLEMENTATION = 0x04, E_DEBUG = 0x80
//	};
//
//
//	union {
//		struct BITMAP_INFO_HEADER {
//			ui32 biHeaderSize;
//			i32  biWidth;
//			i32  biHeight;
//			i16  biPlanes;
//			i16  biBitCount;
//			ui32 biCompression;
//			ui32 biByteSize;
//			i32  biXPPM;
//			i32  biYPPM;
//			ui32 biClrUsed;
//			ui32 biClrImportant;
//		} BI;
//
//		struct BITMAP_V5_HEADER {
//			ui32        headerSize;
//			i32         width;
//			i32         height;
//			ui16        planes;
//			ui16        bitCount;
//			ui32        compression;
//			ui32        byteSize;
//			i32         xPPM;
//			i32         yPPM;
//			ui32        clrUsed;
//			ui32        clrImportant;
//			ui32        redMask;
//			ui32        greenMask;
//			ui32        blueMask;
//			ui32        alphaMask;
//			ui32        CSType;
//
//			struct {
//
//				i32 xRed;
//				i32 yRed;
//				i32 zRed;
//				//------------------------------
//				i32 xGreen;
//				i32 yGreen;
//				i32 zGreen;
//				//------------------------------
//				i32 xBlue;
//				i32 yBlue;
//				i32 zBlue;
//
//			} Endpoints;
//
//			ui32        gammaRed;
//			ui32        gammaGreen;
//			ui32        gammaBlue;
//			ui32        intent;
//			ui32        profileData;
//			ui32        profileSize;
//			ui32        reserved;
//		} V5;
//	};
//	ui8* data = new ui8[1];
//
//	/*TODO: change public to here */
//
//	~Image() {
//		if (data != 0) {
//			delete[] data;
//		}
//	}
//
//	/*---- setting functions ----*/
//
//	/*takes a path to a image file and gets the underlying bitmap*/
//	void ReadDecodeImage(const char* path) {
//		std::string pathStr = path;
//		std::ifstream file(path, std::ios::binary | std::ios::ate);
//
//		if (file) {
//			ui32 fileSize = file.tellg();
//			file.seekg(0, file.beg);
//
//			ui8 possibleError = 0;
//
//			if (!pathStr.substr(pathStr.size() - 3, 3).compare("bmp")) {
//				possibleError = BMPDecoding(file, fileSize);
//			}
//			else if (!pathStr.substr(pathStr.size() - 3, 3).compare("png")) {
//				possibleError = PNGDeconding(file, fileSize);
//			}
//
//			if (possibleError) error |= possibleError;
//		}
//		else {
//			error |= E_FILE;
//		}
//	}
//
//	/*---- consult functions ----*/
//
//	/*Get error info*/
//	ui8 GetError() { return error; }
//
//private:
//
//	//---------------------------------------------------------------------------------------------------
//	// //------------------------------------------------------------------------------------------------
//	// 
//	// PNG DECODING FUNCTIONS
//	// 
//	// //------------------------------------------------------------------------------------------------
//	//---------------------------------------------------------------------------------------------------
//
//
//	int PNGDeconding(std::ifstream& file, ui32 fileSize) {
//		return E_IMPLEMENTATION;
//	}
//
//
//	//---------------------------------------------------------------------------------------------------
//	// //------------------------------------------------------------------------------------------------
//	// 
//	// BMP DECODING FUNCTIONS
//	// 
//	// //------------------------------------------------------------------------------------------------
//	//---------------------------------------------------------------------------------------------------
//
//	int BMPDecoding(std::ifstream& file, ui32 fileSize) {
//		std::string aux;
//		char buff[256] = { 0 };
//
//		if (fileSize < 14) return E_FILE;
//
//		file.read(buff, 14);
//
//		aux = { buff };
//
//		if (aux.length() >= 2 && !aux.substr(0, 2).compare("BM")) {
//			ui32 size = *((ui32*)&buff[2]);
//
//			if (fileSize < size) return E_FILE;
//
//			ui32 offset = *((ui32*)&buff[10]);
//
//			if (offset >= size) return E_ENCODING;
//
//			file.read(buff, 4);
//			ui32 headerSize = *((ui32*)buff);
//
//			if ((!headerSize) || (headerSize + 14 > offset)) return E_ENCODING;
//
//			file.read(buff + 4, (long long)headerSize - 4);
//
//			if (headerSize == 124) {
//				memcpy((void*)&V5, (void*)buff, sizeof(V5));
//
//				//------------------------------------------
//				// aun falta para que esto salga bien
//				//------------------------------------------
//
//				switch (V5.compression) {
//				case RGB:
//				{
//					switch (V5.bitCount) {
//					case 1:
//						RGB1BitDataSet(file);
//						break;
//
//					case 2:
//					{
//
//					}
//
//					break;
//
//					default:
//						return E_ENCODING;
//					}
//				}
//				break;
//				case BITFIELDS:
//				{
//					if (V5.bitCount == 32) {
//						delete[] data;
//
//						data = new ui8[V5.byteSize];
//
//						file.read((char*)data, V5.byteSize);
//					}
//				}
//				break;
//				case RLE8:
//				case RLE4:
//				case VOI_BM_JPEG:
//				case VOI_BM_PNG:
//					break;
//				default:
//					return E_ENCODING;
//				}
//			}
//		}
//
//		return 0;
//	}
//
//	void RGB1BitDataSet(std::ifstream& file) {
//		char* bitData = new char[V5.byteSize];
//
//		file.read(bitData, V5.byteSize);
//
//		i64 arrSize = std::abs((i64)V5.width * V5.height);
//
//		delete[] data;
//		data = new ui8[arrSize * 4];
//
//		Pixel* pixData = (Pixel*)data;
//
//		ui8 mask = 0x80;
//		for (i64 i = 0; i < V5.byteSize; i++) {
//			ui8 currByte = bitData[i];
//			for (i64 j = 0; (mask >> j) > 0; j++) {
//				if (((i * 8) + j) >= arrSize) goto bit_copy_ended;
//
//				if (((mask >> j) & currByte) > 0) pixData[(i * 8) + j] = { 0xFF,0xFF,0xFF };
//				else pixData[(i * 8) + j] = { 0,0,0 };
//			}
//		}
//
//	bit_copy_ended:;
//
//	};
//
//};







