#pragma once
/*
    VideoMode defines a video mode (width,height,bits per pixel)
*/
class VideoMode
{
public:
	/*
        Default constructor
        This constructor initializes all members to 0.
    */
	VideoMode();

	/*
        Construct the video mode with its attributes

        Params:
        modeWidth			Width in pixels
        modeHeight			Height in pixels
        modeBitsPerPixel	Pixel depths in bits per pixel
    */
	VideoMode(unsigned short _modeWidth,
			  unsigned short _modeHeight,
			  unsigned short _modeBitsPerPixel = 32);

	/*
        Get the current desktop video mode

        return:
        Current dekstop video mode
    */
	static VideoMode GetDesktopMode();

	/*
        Get the current mode width

        return:
        Mode width
    */
	unsigned short GetWidth() const;

	/*
        Get the current mode height

        return:
        Mode height
    */
	unsigned short GetHeight() const;

	/*
        Get the current mode Bits Per Pixel

        return
        Mode pixel depths in bits per pixel
    */
	unsigned short GetBitsPerPixel() const;

private:
	unsigned short m_width;
	unsigned short m_height;
	unsigned short m_bitsPerPixel;
};