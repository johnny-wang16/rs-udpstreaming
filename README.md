# rs-udpstreaming
This repository contains an example application that streams both RGB and 16-bit depth data simultaneously from the intel
realsense device. It mainly uses Gstreamer(version 1.8.3) and Intel RealSense SDK. I tested on my system which runs Ubuntu 16.04.
The setup instruction is pretty long so bear with me for awhile and report issues if anything abnormal happens.

#### Set up instructions for the host computer (the one that's attached to the intel realsene camera):
##### A. Enviornment Setup:

1. Set up virtual enviornment through anaconda and activate it. (Python version 3.5 is recommended because Opencv is only compatible with python 3.5)
- I tried python 3.7 and 3.8 but was getting errors during runtime from OpenCV saying that the module python version mismatched
- I also tried to set up virtual enviornment with virtualenv for some reason, the OpenCV cmake script cannot detect Python3. It's probably possible
to set up with virtualenv but I didn't have time for it.

2. Getting Numpy by `sudo apt-get` or `pip`.

##### B. Install Gstreamer from source:
The specific instructions I referenced is [here](https://blog.csdn.net/weixin_30483697/article/details/101178427?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task).

- I recommend Gstreamer version 1.8.3.
- I tried 1.16.0 but seems like it is not compatible with OpenCV. I kept getting error during the sudo make stage 
even if the cmake step works fine. 

###### NOTE: Aside from the package described in the above instruction, you also need to download gst-rtsp-server-1.xx.xx.tar.xz and compile it the same way as other packages (ex:gst-plugins-ugly-1.16.0.tar.xz , gst-libav-1.16.0.tar.xz)
- I am surprised there's a lack of English instructions to build Gstreamer from source. I'll make one in the future.

##### C. Install and Compile OpenCV from source with Gstreamer support
- The speicific instruction I referenced is [here](https://medium.com/@galaktyk01/how-to-build-opencv-with-gstreamer-b11668fa09c)
###### NOTE: You can skip to step 4 as steps 1-3 is completed in A and B.
###### NOTE: On Ubuntu system, there's one step that you need to modify in the above instruction. In step 5.Building,you need to change the double quotes in the cmake insturction to single quote. So the cmake command would be:
```
cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D INSTALL_PYTHON_EXAMPLES=ON \
-D INSTALL_C_EXAMPLES=OFF \
-D PYTHON_EXECUTABLE=$(which python3) \
-D BUILD_opencv_python2=OFF \
-D CMAKE_INSTALL_PREFIX=$(python3 -c 'import sys; print(sys.prefix)') \
-D PYTHON3_EXECUTABLE=$(which python3) \
-D PYTHON3_INCLUDE_DIR=$(python3 -c 'from distutils.sysconfig import get_python_inc; print(get_python_inc())') \
-D PYTHON3_PACKAGES_PATH=$(python3 -c 'from distutils.sysconfig import get_python_lib; print(get_python_lib())') \
-D WITH_GSTREAMER=ON \
-D BUILD_EXAMPLES=ON ..
```
Otherwise, there will be a syntax error.

###### NOTE: After you finish building OpenCV, make sure to test if the setup is successful by doing the following on the terminal:
```python
import cv2
print(cv2.getBuildInformation())
```
###### The GStreamer option should be "Yes" and there should be a section for Python3.  
If your system cannot locate OpenCV, try moving the .so file to the source code folder. Check out step 5 of the link 
[here](https://www.pyimagesearch.com/2018/05/28/ubuntu-18-04-how-to-install-opencv/) to learn more about this step.

##### D. Get Intel Realsene SDK
The specific link I referenced is [here](https://github.com/IntelRealSense/librealsense/blob/master/doc/installation.md).Make sure to read through it and follow every single step. The instructions listed is pretty clear and should be able to guide
you through the installation concepts.

#### Set up instructions for the client computer (the one that's going to recieve streams through UDP connection):
You will still need python, Gstreamer and OpenCV for the client. You don't need the realsense SDK though.

Congratulations! That's it for the installation process.

#### Running the code:
1. Make sure to change the IP address in both `stream.cpp` and `reciever.py`

2. If you are not streaming from an intel realsense camera and streaming from a pre-recorded rosbag file instead (maybe for testing purposes), uncomment the line:
`rs_cfg.enable_device_from_file("/home/johnny/Downloads/d435i_sample_data/d435i_walk_around.bag"); `
and comment out the lines: `rs_cfg.enable_stream(RS2_STREAM_DEPTH, WIDTH, HEIGHT, RS2_FORMAT_Z16, FRAME); rs_cfg.enable_stream(RS2_STREAM_COLOR, WIDTH, HEIGHT, RS2_FORMAT_RGB8, FRAME);`
, vice versa.

3. Start the receiver first and then the sender.



