import sys, getopt, Image, io, urllib

def main(argv):
    inputfile = ''
    width = ''
    arguments = False
    debug = False;
    try:
        opts, args = getopt.getopt(argv, "i:d")
    except getopt.GetoptError:
        print 'image-processing.py -i <inputfile> -d <debug>'
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

    image_file = image_file.convert('1') # convert image to black and white
    (width,height) = image_file.size
    half = (width/2) -1

    leftscore = 0
    rightscore = 0

    for i in range(0, height-2):
        for j in range(0, half):
            leftscore += image_file.getpixel((j,i))
        for j in range(half+1, width-1):
            rightscore += image_file.getpixel((j,i))

    if (debug) :
        print("Left Score: " + str(leftscore))
        print("Right Score: " + str(rightscore))

    delta = 1.05

    if leftscore > rightscore:
        score = float(leftscore) / float(rightscore)
        if (debug) :
            print score
        if score > delta:
            print("RIGHT")
        else:
            print("STRAIGHT")
    else:
        score = float(rightscore) / float(leftscore)
        if (debug) :
            print score
        if score > delta:
            print("LEFT")
        else:
            print("STRAIGHT")

if __name__ == "__main__":
   main(sys.argv[1:])
        
