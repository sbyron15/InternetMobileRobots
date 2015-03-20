import sys, getopt, Image, io, urllib

def main(argv):
    inputfile = ''
    width = ''
    arguments = False
    debug = False;
    try:
        opts, args = getopt.getopt(argv, "i:d")
    except getopt.GetoptError:
        print 'follow-line.py -i <inputfile> -d <debug>'
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

    #image_file = image_file.convert('1') # convert image to black and white
    (width,height) = image_file.size
    leftBoundary = 2*(width/5)
    rightBoundary = width-leftBoundary

    leftscore = 1
    rightscore = 1
    centerscore = 1

    pix = image_file.load();

    for i in range(0, height-2):
        for j in range(0, leftBoundary):
            (r,g,b) = pix[j,i]
            if r<100 and g<100 and b<100:
                leftscore+=1
                if (debug) :
                        print (r,g,b)
        for j in range(leftBoundary+1, rightBoundary):
            (r,g,b) = pix[j,i]
            if r<100 and g<100 and b<100:
                centerscore+=1
                if (debug) :
                        print (r,g,b)
        for j in range(rightBoundary+1, width-1):
            (r,g,b) = pix[j,i]
            if r<100 and g<100 and b<100:
                rightscore+=1
                if (debug) :
                        print pix[j,i]

    if (debug) :
        print("Left Score: " + str(leftscore))
        print("Center Score: " + str(centerscore))
        print("Right Score: " + str(rightscore))

    if (centerscore+leftscore+rightscore)<600:
        print("STOP")
    elif centerscore>leftscore and centerscore>rightscore:
        print("FORWARD")
    elif leftscore > rightscore:
        print("LEFT")
    else:
        print("RIGHT")

if __name__ == "__main__":
   main(sys.argv[1:])
        
