import cv2
import numpy as np
import argparse


ap = argparse.ArgumentParser()
ap.add_argument("-i", "--image", required = True, help = "Path to the image")
args = vars(ap.parse_args())

fname = args['image']
print("Reading image " + fname + " ... ")
img = cv2.imread(fname)
output = img.copy()

print("Converting to GrayScale...")
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

print("Bluring ...")
#gray = cv2.medianBlur(gray, 5)
gray = cv2.GaussianBlur(gray, (5,5), 1.5)
cv2.imshow('original', img)
cv2.imshow('gray', gray)
print("Detecting circles...")
circles = cv2.HoughCircles(gray, cv2.cv.CV_HOUGH_GRADIENT, 2, 300,
#        param1=100, param2=200, minRadius=200, maxRadius=500)
        param1=100, param2=200, minRadius=200, maxRadius=400)
print("Done...")

if circles is not None:
    print("converting numbers...")
    circles = np.uint16(np.around(circles))
    idx = 0
    for i in circles[0,:]:
        print("drawing image ... {0}", idx)
        cv2.circle(output,(i[0],i[1]),i[2],(0,255,0),2)
        cv2.circle(output,(i[0],i[1]),2,(0,0,255),3)
        idx += 1
else:
    print("no circles detected")

cv2.imshow('detected', output)
cv2.waitKey(0)
cv2.destroyAllWindows()

