import serial #communication with arduino
import threading #reading from sensor and arduino in background
import numpy as np #used to read the question text files
#####FLASK#########
from flask import Flask, render_template, redirect, url_for
from turbo_flask import Turbo #to update variables in static HTML
#####MPR121##########
import adafruit_mpr121 #capacititive sensor 
import board #for python to use the sensor, circuitpython
import busio #to use the I2C protocol 
#####LED's########
from rpi_ws281x import * #LED library from adafruit
import argparse #write to the LED strip
#####PAGE REFRESH########
import pyautogui #to simulate page refresh by pressing F5
from selenium import webdriver
#####QR CODE#########
import qrcode #Creates a qr code from a link
from PIL import Image #used to save qr code as image  
import base64 #used to encode image and send to HTML
import io #used to encode image and send to HTML
    


##===============VARIABLES==================================##
state = "awaitingPickup" # awaitingPickup, awaitingBall, base1, base2, reveal
lastTouchIndex = "0.0" 
question = ""
answer = "0.0" #float from 1 to 10 
selectedAnswer = "0.0" #position in cap array int from 1 to 10
selecOp = ""
questionloop = 0
revealtext = ""
region = ""
difference = ""
value = ""


listBaseOne = np.genfromtxt(r'QuestionsBaseOne.txt', dtype=str, delimiter=";")
listBaseTwo = np.genfromtxt(r'QuestionsBaseTwo.txt', dtype=str, delimiter=";")

oplist = listBaseOne[0][3:13] ##takes a default from base1 q1 so that there's no error from turbo flask bc empty

##==============HARDWARE====================================##
# Create I2C bus.
i2c = busio.I2C(board.SCL, board.SDA)

# Create MPR121 object, capacatitive touch.
mpr121 = adafruit_mpr121.MPR121(i2c)


# LED strip configuration:
LED_COUNT      = 10      # Number of LED pixels.
LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 20     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53


# Create NeoPixel object with appropriate configuration.
strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
strip.begin()

##===================LED Functions=========================================##
def colorWipe(strip, color, pos):
    """Wipe color across display a pixel at a time."""
    for i in range(pos):
        strip.setPixelColor(i, color)
        strip.show()

def colorWipeAll(strip, pos):
    """Wipe color across display a pixel at a time."""
    for i in range(pos, 10):
        strip.setPixelColor(i, Color(0, 0, 0))
        strip.show()



##======================COM WITH ARDUINO==================================##

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser.reset_input_buffer()

##======================SELENIUM==========================================##

driver = webdriver.Chrome()
#"executable_path = '/usr/lib/chromedriver"
##============================FLASK==================================##

app = Flask(__name__)
turbo = Turbo(app)


##==================THREADS================================================##

@app.before_first_request
def activate_job():
    def capTouchBar(): #update cap touch bar graph
        global selecOp, oplist, lastTouchIndex
        with app.app_context():
            while True:
                if mpr121[0].value: #if channel 0 is True (touched)
                    selecOp = oplist[0] #select the first option from the question txt file
                    lastTouchIndex = selecOp #sets that option as the last touched 
                    colorWipeAll(strip, 1) #turns off all LED's with a position higher than 1
                    colorWipe(strip, Color(0, 0, 255), 1) #turns on all LED's until position 1
                    print(0) #prints to the terminal, used for debugging, not seen by user
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
                    #pushes the variable selecOp from line 124 to the HTML page

                elif mpr121[1].value:
                    selecOp = oplist[1]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 2)
                    colorWipe(strip, Color(0, 0, 255), 2)
                    print(1)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[2].value:
                    selecOp = oplist[2]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 3)
                    colorWipe(strip, Color(0, 0, 255), 3)
                    print(2)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
             

                elif mpr121[3].value:
                    selecOp = oplist[3]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 4)
                    colorWipe(strip, Color(0, 0, 255), 4)
                    print(3)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
                    

                elif mpr121[4].value:
                    selecOp = oplist[4]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 5)
                    colorWipe(strip, Color(0, 0, 255), 5)
                    print(4)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[5].value:
                    selecOp = oplist[5]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 6)
                    colorWipe(strip, Color(0, 0, 255), 6)
                    print(5)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[6].value:
                    selecOp = oplist[6]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 7)
                    colorWipe(strip, Color(0, 0, 255), 7)
                    print(6)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[7].value:
                    selecOp = oplist[7]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 8)
                    colorWipe(strip, Color(0, 0, 255), 8)
                    print(7)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[8].value:
                    selecOp = oplist[8]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 9)
                    colorWipe(strip, Color(0, 0, 255), 9)
                    print(8)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                elif mpr121[9].value:
                    selecOp = oplist[9]
                    lastTouchIndex = selecOp
                    colorWipeAll(strip, 10)
                    colorWipe(strip, Color(0, 0, 255), 10)
                    print(9)
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))
            

                else: 
                    selecOp = lastTouchIndex 
                    turbo.push(turbo.replace(render_template('selectedoption.html'), 'selecOp'))    



    def comWithArduino(): #update global state variable 
        global state #access to the global state variable

        with app.app_context():
            while True:
                if ser.in_waiting > 0: #if the arduino sends something 
                    #we recieve bytes so we decode them to a string
                    ardustate = ser.readline().decode('utf-8').rstrip() 
                    state = ardustate #updates the global stae
                    print(state) #prints state to console for debugging 

                    if state == "awaitingPickup":
                        driver.get("http://127.0.0.1:5000/awaitingPickup")

                    elif state == "awaitingBall":
                        driver.get("http://127.0.0.1:5000/awaitingBall")

                    elif state == "base1":
                        driver.get("http://127.0.0.1:5000/question")

                    elif state == "base2":
                        driver.get("http://127.0.0.1:5000/question")

                    elif state == "reveal":
                        driver.get("http://127.0.0.1:5000/reveal")



    # Starts the threads
    thread = threading.Thread(target=capTouchBar) 
    thread.start()
    thread2 = threading.Thread(target=comWithArduino) 
    thread2.start()


##From flask, updates variables in a html tempalte without reloading page, used to aniamte bar 
@app.context_processor
def inject_load():

    return {'selecOp': selecOp}

##=================APP ROUTES============================================##

@app.route("/awaitingPickup/") #pick up ball
def awaitingPickup():

    return render_template("default.html")
   

@app.route("/awaitingBall/") #pick up ball
def awaitingBall():

    global lastTouchIndex

    colorWipeAll(strip, 0) #Wipes all led for reset 
    lastTouchIndex = "0"


    return render_template("awaitingBall.html")


##=========================QUESTION PAGE=============================##

@app.route("/question/")
def question():
    
    global  selectedAnswer, answer, state, oplist, listBaseOne, listBaseTwo, questionloop, revealtext, region, lastTouchIndex, question, value, selecOp
    
    
    ## Depending on state, select questions from base1 or base2 
    if state == "base1" : 
        question = listBaseOne[questionloop][0]
        revealtext = listBaseOne[questionloop][1]  
        answer = listBaseOne[questionloop][2] 
        oplist = listBaseOne[questionloop][3:13]
        value = listBaseOne[questionloop][14]
        region = "Culture"


    elif state == "base2" :
        question = listBaseTwo[questionloop][0]
        revealtext = listBaseTwo[questionloop][1]  
        answer = listBaseTwo[questionloop][2] 
        oplist = listBaseTwo[questionloop][3:13]
        value = listBaseTwo[questionloop][14]
        region = "Transport"

    else : 
        print("Error in reading base 1 or 2 state in question state")


    ##Loops thhrough the questions and then resets when all questions have been displayed
    if questionloop +1 >= len(listBaseOne) : #+1 because len starts at 1 and not 0 
        questionloop = 0

    else : 
        questionloop = questionloop +1


    ##Last cap touch pad to be touched before state changes is the selected answer 
    selectedAnswer = lastTouchIndex



    return render_template("question.html", question = question, options = oplist, region = region, value = value)





##=======================REVEAL PAGE===============================##
@app.route("/reveal/")
def reveal():
    
    global question, selectedAnswer, answer, revealtext, listBaseOne, listBaseTwo, questionloop, difference, value, lastTouchIndex

    
    selectedAnswer = lastTouchIndex
    encodedRevealText = revealtext.replace(" ", "%20")

    if float(selectedAnswer)-float(answer) < 0 :
        lessmore = "more"
        difference = str(float(answer) - float(selectedAnswer))

    elif float(selectedAnswer)-float(answer) > 0:
        lessmore = "less"
        difference = str(float(selectedAnswer) - float(answer))

    elif float(selectedAnswer)-float(answer) == 0:
        lessmore = ""
        value = ""
        difference = "exactly not more or less"

    else : 
        lessmore = "error in calc answer to selectedAnswer"

    

    ##Need to use one % more at each %20 in order to escape the first %s
    ##So coloring is fucked up
    qrlink = "https://www.aalborg.dk/51934?view=cm&68c963d3-3785-410b-bf46-294cd436ff8c=%s%%20,%%20which%%20is%%20%s%%20%s%%20%s%%20than%%20the%%20%s%%20%s%%20you%%20have%%20chosen.&fs=1.aspx" % (encodedRevealText, difference, value, lessmore, selectedAnswer, value)



    ##Making the qr code 
    img = qrcode.make(qrlink)

    type(img)

    ##Fills ins variables to name based on what user chose 
    imgname = str(question) + selectedAnswer

    ##Creates the file name replaces %s with var imgname
    filename = "%s.png" % imgname

    ##Saves the file 
    img.save(filename)

    ##Open image to display 
    
    im = Image.open(filename)
    
    ##Converts the image to be passed as variable 
    data = io.BytesIO()
    
    im.save(data, "PNG")
    
    encoded_img_data = base64.b64encode(data.getvalue())




    return render_template("reveal.html", img_data=encoded_img_data.decode('utf-8'), revealtext = revealtext, selectedAnswer = selectedAnswer, lessmore = lessmore, answer = answer, difference = difference, value = value)
            







##=====================PROGRAM================================================##



if __name__ == "__main__":

    app.run()
    driver.get("http://127.0.0.1:5000/awaitingPickup")

