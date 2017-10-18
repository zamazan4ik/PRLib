PRLib - Pre-Recognition Library.

The main aim of the library - prepare image for recogntion. Image processing can really help to improve recognition quality.

*Features:*
* Auto crop image
* Background normalization
* Binarization (Global Otsu, Local Otsu, Sauvola, Feng, Niblack, Wolf-Jolion, NICK, FBCITB, Adaptive)
* Blur detection (LAPM, LAPV, TENG, GLVN)
* Color balance
* Crop
* Deblur (Gaussian-based)
* Denoise (Non-local Means Denoising algorithm, Salt-Pepper)
* Deskew
* Find angle and orientation of an image
* Gamma correction
* Hole punch removal
* Line removal
* Perspective warp removal
* Thinning (Zhang-Suen, Guo-Hall)
* White balance (Simple, Grayworld)

*Dependencies:* OpenCV, Leptonica.

*How to build:*
1) Go to the directory with source code
2) Make directory for build:
```
    mkdir build
```
3) Run cmake:
```
    cmake ..
```    
4) Run make:
```
    make
```    
