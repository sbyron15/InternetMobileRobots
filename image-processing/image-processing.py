import sys, getopt

def main(argv):
    inputfile = ''
    width = ''
    try:
        opts, args = getopt.getopt(argv, "i:w:")
    except getopt.GetoptError:
        print 'image-processing.py -i <inputfile> -w <width>'
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-i':
            inputfile = arg;
        elif opt == '-w':
            width = int(arg);

    image = open(inputfile, 'r')
    j = 0
    leftscore = 0
    rightscore = 0
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

if __name__ == "__main__":
   main(sys.argv[1:])
        
