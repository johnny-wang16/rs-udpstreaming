# # MIT License
# # Copyright (c) 2019 JetsonHacks
# # See LICENSE for OpenCV license and additional information

# # https://docs.opencv.org/3.3.1/d7/d8b/tutorial_py_face_detection.html
# # On the Jetson Nano, OpenCV comes preinstalled
# # Data files are in /usr/sharc/OpenCV
# import numpy as np
# import cv2
# import pyrealsense2 as rs
# from multiprocessing import Process

# # gstreamer_pipeline returns a GStreamer pipeline for capturing from the CSI camera
# # Defaults to 1280x720 @ 30fps 
# # Flip the image by setting the flip_method (most common values: 0 and 2)
# # display_width and display_height determine the size of the window on the screen

# # def gstreamer_pipeline(capture_width=3280, capture_height=2464, output_width=224, output_height=224, framerate=21, flip_method=0) :   
# #         return 'nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=%d, height=%d, format=(string)NV12, framerate=(fraction)%d/1 ! nvvidconv flip-method=%d ! nvvidconv ! video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! videoconvert ! appsink' % (
# #                 capture_width, capture_height, framerate, flip_method, output_width, output_height)
# fps = 30.
# frame_width = 640
# frame_height = 480

# client_ip='127.0.0.1'
# # cap = cv2.VideoCapture("v4l2src device=/dev/video=0 ! video/x-raw,framerate=30/1 ! appsink")
# cap = cv2.VideoCapture('udpsrc port=5200 caps=application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264,payload=(int)96!rtph264depay!decodebin!videoconvert!appsink',cv2.CAP_GSTREAMER)

# if not cap.isOpened():
# 	print("cannot caputure from camera. Exiting")
# 	quit()

# gst_str_rtp = 'appsrc ! videoconvert ! x264enc ! h264parse ! rtph264pay pt=96 ! udpsink host=127.0.0.1 port=5000'

# def send():
#     cap_send =cv2.VideoWriter(gst_str_rtp, 0, fps, (frame_width, frame_height), True)
    
#     # if not cap_send.isOpened():
#     #     print('VideoCapture or VideoWriter not opened')
#     #     exit(0)

#     while True:
#         ret,frame = cap_send.read()

#         if not ret:
#             print('empty frame')
#             break

#         out_send.write(frame)

#         cv2.imshow('send', frame)
#         if cv2.waitKey(1)&0xFF == ord('q'):
#             break

#     cap_send.release()

# # def sendrs():
# #     out_send = cv2.VideoWriter('appsrc ! videoconvert ! video/x-raw, format=(string)BGRx, width=(int)1280, height=(int)720, framerate=(fraction)30/1 ! videoconvert ! video/x-raw, format=(string)I420 ! omxh264enc control-rate=2 bitrate=4000000 ! video/x-h264, stream-format=byte-stream ! rtph264pay mtu=1400 ! udpsink host=%s port=5001 sync=false async=false'%(client_ip),cv2.CAP_GSTREAMER,0,30,(1280,720), True)

# #     pipe = rs.pipeline()
# #     cfg = rs.config()
# #     #cfg.enable_stream(rs.stream.gyro)
# #     cfg.enable_stream(rs.stream.color, 1280, 720, rs.format.bgr8, 30) # color camera
# #     pipe.start(cfg)
# #     while True:
# #         frames = pipe.wait_for_frames()
# #         color_frame = frames.get_color_frame()
# #         img = np.asanyarray(color_frame.get_data())
# #         #cv2.imshow('send1', img)
# #         #if cv2.waitKey(1)&0xFF == ord('q'):
# #             #break

# #         out_send.write(img)
# #     out_send.release()

# if __name__ == '__main__':
#     #send()
#     #sendrs()
#     s = Process(target=send)
#     # r = Process(target=sendrs)
#     s.start()
#     # r.start()
#     s.join()
#     # r.join()

#     cv2.destroyAllWindows()



import time
import cv2
from multiprocessing import Process

# # Cam properties
# fps = 30.
# frame_width = 640
# frame_height = 480
# # Create capture
# cap = cv2.VideoCapture('videotestsrc ! video/x-raw,framerate=30/1 ! videoscale ! videoconvert ! appsink', cv2.CAP_GSTREAMER)

# if not cap.isOpened():
#     print("Cannot open camera. Exiting.")
#     quit()

# # cap = cv2.VideoCapture(0)

# # Set camera properties
# # cap.set(cv2.CAP_PROP_FRAME_WIDTH, frame_width)
# # cap.set(cv2.CAP_PROP_FRAME_HEIGHT, frame_height)
# # cap.set(cv2.CAP_PROP_FPS, fps)

# # Define the gstreamer sink
# # gst_str_rtp = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=5000"
# gst_str_rtp = "appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=5000"


# # Check if cap is open
# # if cap.isOpened() is not True:
# #     print("Cannot open camera. Exiting.")
# #     quit()

# # Create videowriter as a SHM sink
# out = cv2.VideoWriter(gst_str_rtp, 0, fps, (frame_width, frame_height), True)

# # Loop it
# while True:
#     # Get the frame
#     ret, frame = cap.read()
#     # Check
#     if ret is True:
#         # Flip frame
#         frame = cv2.flip(frame, 1)
#         # Write to SHM
#         out.write(frame)
#         print("in")
#     else:
#         print("Camera error.")
#         time.sleep(10)

# cap.release()

def send():
    cap_send = cv2.VideoCapture('v4l2src device=/dev/video0 ! video/x-raw,framerate=30/1 ! videoscale ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
    out_send = cv2.VideoWriter('appsrc ! videoconvert ! x264enc tune=zerolatency ! rtph264pay pt=96 ! udpsink host=127.0.0.1 port=5000',cv2.CAP_GSTREAMER,0, 30, (640,480), True)

    if not cap_send.isOpened() or not out_send.isOpened():
        print('VideoCapture or VideoWriter not opened')
        exit(0)

    while True:
        ret,frame = cap_send.read()

        if not ret:
            print('empty frame')
            break

        out_send.write(frame)

        # cv2.imshow('send', frame)
        # if cv2.waitKey(1)&0xFF == ord('q'):
        #     break

    cap_send.release()
    out_send.release()

# def receive():
#     # cap_receive = cv2.VideoCapture('udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
#     cap_receive = cv2.VideoCapture('udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink', cv2.CAP_GSTREAMER)

#     if not cap_receive.isOpened():
#         print('VideoCapture not opened')
#         exit(0)

#     while True:
#         ret,frame = cap_receive.read()

#         if not ret:
#             print('empty frame')
#             break

#         cv2.imshow('receive', frame)
#         if cv2.waitKey(1)&0xFF == ord('q'):
#             break

#     #cap_receive.release()
def receive1():
    # cap_receive = cv2.VideoCapture('udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
    # cap_receive = cv2.VideoCapture('udpsrc port=5001 caps = "application/x-rtp, clock-rate=(int)90000, encoding-name=(string)X-GST, payload=(int)96" ! rtpgstdepay ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
    # cap_receive = cv2.VideoCapture('udpsrc port="5000" caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)RAW, sampling=(string)RGB, width=(string)640, height=(string)480, payload=(int)96" ! rtpvrawdepay ! videoconvert ! queue ! appsink', cv2.CAP_GSTREAMER)
    cap_receive = cv2.VideoCapture('udpsrc port=5001 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! appsink', cv2.CAP_GSTREAMER)

    if not cap_receive.isOpened():
        print('VideoCapture not opened')
        exit(0)

    while True:
        ret,frame = cap_receive.read()

        if not ret:
            print('empty frame')
            break

        cv2.imshow('receive1', frame)
        if cv2.waitKey(1)&0xFF == ord('q'):
            break

def receive2():
    cap_receive = cv2.VideoCapture('udpsrc port=5000 caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
    # cap_receive = cv2.VideoCapture('udpsrc port=5001 caps = "application/x-rtp, clock-rate=(int)90000, encoding-name=(string)X-GST, payload=(int)96" ! rtpgstdepay ! videoconvert ! appsink', cv2.CAP_GSTREAMER)
    # cap_receive = cv2.VideoCapture('udpsrc port="5000" caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)RAW, sampling=(string)RGB, width=(string)640, height=(string)480, payload=(int)96" ! rtpvrawdepay ! videoconvert ! queue ! appsink', cv2.CAP_GSTREAMER)

    if not cap_receive.isOpened():
        print('VideoCapture not opened')
        exit(0)

    while True:
        ret,frame = cap_receive.read()

        if not ret:
            print('empty frame')
            break

        cv2.imshow('receive2', frame)
        if cv2.waitKey(1)&0xFF == ord('q'):
            break

if __name__ == '__main__':
    # s = Process(target=send)
    r1 = Process(target=receive1)
    r2 = Process(target=receive2)

    # s.start()
    r1.start()
    r2.start()
    # s.join()
    r1.join()
    r2.join()

    cv2.destroyAllWindows()
