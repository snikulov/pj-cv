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
gray = cv2.GaussianBlur(gray, (5,5), sigmaX=1.5, sigmaY=1.5)

#cv2.imshow('original', img)
#cv2.imshow('gray', gray)
print("Detecting circles...")

heigth, width = gray.shape[:2]
circles = cv2.HoughCircles(gray, method=cv2.HOUGH_GRADIENT, dp=2, minDist=300,
        param1=50, param2=300, minRadius=350, maxRadius=650)

print("Done...")

head = "roi"
if circles is not None:
    print("converting numbers...")
    circles = np.uint16(np.around(circles))
    idx = 0
    for i in circles[0,:]:
        print("drawing image ... {0}", idx)
        cv2.circle(output,(i[0],i[1]),i[2]+20,(0,255,0),2)
        cv2.circle(output,(i[0],i[1]),2,(0,0,255),3)
        idx += 1
#  roi - (y0, y1 : x0, x1)
        y0 = i[1]-(i[2]+100)
        if y0 < 0:
            y0 = 0
        heigth, width = output.shape[:2]
        y1 = i[1]+(i[2]+100)
        if y1 > heigth:
            y1 = heigth
        x0 = i[0]-(i[2]+100)
        if x0 < 0:
            x0 = 0
        x1 = i[0]+(i[2]+100)
        if x1 > width:
            x1 = width
        roi = output[y0:y1, x0:x1]
        cv2.imshow(head + str(idx), roi)
else:
    cv2.imshow('detected', output)
    print("no circles detected")

cv2.waitKey(0)
cv2.destroyAllWindows()

