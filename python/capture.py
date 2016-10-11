import cv2
import numpy as np
import argparse


ap = argparse.ArgumentParser()
ap.add_argument('-u', '--url', required = True, help = 'URL to network camera')
args = vars(ap.parse_args())

u = args['url']
print('Reading stream from: ' + u)

cap = cv2.VideoCapture(u)
ecount = 0
while cap.isOpened():
    ret, frame = cap.read()
    if ret :
        cv2.imshow('frame',frame)
    else :
        ecount += 1
        print('error reading frame ' + str(ecount))
    if cv2.waitKey(1) == 27:
        break

cap.release()
cv2.destroyAllWindows()

