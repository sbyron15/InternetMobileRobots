import sys, getopt, Image, io, urllib

def main(argv):
    inputfile = ''
    width = ''
    arguments = False
    debug = False;
    try:
        opts, args = getopt.getopt(argv, "i:d")
    except getopt.GetoptError:
        print 'follow-green.py -i <inputfile> -d <debug>'
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-i':
            inputfile = arg
            arguments = True
        if opt == '-d':
            debug = True
        
    if (arguments) :
        image_file = Image.open(inputfile)
    else:
        url = urllib.urlopen("http://localhost:8080/?action=snapshot")
        snapshot = io.BytesIO(url.read())
        image_file = Image.open(snapshot)

    (width,height) = image_file.size
    half = (width/2) -1

    leftscore = 1
    rightscore = 1

    pix = image_file.load();

    for i in range(0, height-2):
        for j in range(0, half):
            (r,g,b) = pix[j,i]
            if ((g-b)>10 and (g-r)>10):
                leftscore += g
                if (debug) :
                    print (r,g,b)
        for j in range(half+1, width-1):
            (r,g,b) = pix[j,i]
            if ((g-b)>10 and (g-r)>10):
                rightscore += g
                if (debug) :
                    print (r,g,b)

    if (debug) :
        print("Left Score: " + str(leftscore))
        print("Right Score: " + str(rightscore))

    delta = 2


    # If there is a VERY SMALL number of green
    # pixels, then we need to stop
    if (leftscore+rightscore)<500:
        print("STOP")

    # If there is a VERY LARGE number of pixels
    # we're right in front of it and need to stop
    elif (leftscore+rightscore)>80000:
        print("STOP")

    # If there is a LARGE number of pixels, then we
    # need to keep going straight, almost there
    elif (leftscore+rightscore)>25000:
        print("FORWARD")

    # If there is a medium number of green pixels,
    # we're probably far away so keep going straight
    elif (leftscore+rightscore)<6000:
        print("FORWARD")

    # Otherwise, take the ratios and see which way to go
    elif leftscore > rightscore:
        score = float(leftscore) / float(rightscore)
        if (debug) :
            print score
        if score > delta:
            print("LEFT")
        else:
            print("FORWARD")
    else:
        score = float(rightscore) / float(leftscore)
        if (debug) :
            print score
        if score > delta:
            print("RIGHT")
        else:
            print("FORWARD")

if __name__ == "__main__":
   main(sys.argv[1:])
        
