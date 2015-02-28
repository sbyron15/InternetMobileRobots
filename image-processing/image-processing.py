image = open('right.pgm', 'r')
j = 0
leftscore = 0
rightscore = 0
width = 640
i = 0
for line in image:
    if j < 3:
        j += 1
        
    else:
        lineSegments = line.split(' ')
        for pixel in lineSegments:
            if pixel.strip() != '':
                if i > (width - 1):
                    i = 0
                    next
                if i < (width/2):
                    leftscore += int(pixel)
                else:
                    rightscore += int(pixel)
                i += 1
        
image.close()
print("Left Score: " + str(leftscore) + "\n")
print("Right Score: " + str(rightscore))

delta = 1.05
if leftscore > rightscore:
    score = float(leftscore) / float(rightscore)
    print score
    if score > delta:
        print("turn right\n")
    else:
        print("go straight\n")
else:
    score = float(rightscore) / float(leftscore)
    print score
    if score > delta:
        print("turn left\n")
    else:
        print("go straight\n")

        
