# -*- coding: utf-8 -*-
"""
Created on Wed Mar 18 02:27:20 2020

@author: weesh
"""
import requests
import sqlite3
import datetime 
import time
import re
import math

clock_db = "__HOME__/smartClock/smartclock.db"
sleep_db = "__HOME__/smartClock/sleeptimes.db"

def normTo100(arr):
    try:
        num = min(arr) / (max(arr)-min(arr))
        return [50*(el/(max(arr)-min(arr))-num)+50 for el in arr] #50 + 50*(el-min(arr))/(max(arr)-min(arr)) for el in arr]
    except:
        return [0]


def timeToStr(f1, f2):
    mins = 60*(math.floor(f2)-math.floor(f1)) + 100*(f2-math.floor(f2) - (f1-math.floor(f1)))
    hours = (int) (mins/60)
    realMins = (int) (mins%60)
    s1 = str(f1)
    s2 = str(f2)
    s1=s1.replace(".", ":")
    while(len(s1) - 1 - s1.index(":") < 2):
        s1 += "0"

    
    s2=s2.replace(".", ":")
    while(len(s2) - 1 - s2.index(":") < 2):
        s2 += "0"

    
    return "Slept at: "+s1+"<br> Woke at:"+s2+"<br>Sleep Duration: "+str(hours)+" hours, "+str(realMins)+" minutes "
    
def request_handler(request):
    
    if(request['method'] == 'POST'):
        try:
            light = float(request['form']['light']);
            temp = float(request['form']['temp']);
            active = int(request['form']['active']);
            sound = float(request['form']['sound']);
            
            conn = sqlite3.connect(clock_db)
            c = conn.cursor()
            c.execute('''CREATE TABLE IF NOT EXISTS senses_table (light float, temp float, active int, float sound, timing timestamp);''')
            
            c.execute('''INSERT into senses_table VALUES (?,?,?,?,?);''', (light, temp, active, sound, datetime.datetime.now()- datetime.timedelta(hours = 1)))
            

            conn.commit()
            conn.close()
            
            timeStr = str(datetime.datetime.now()-datetime.timedelta(hours = 1))
            if(" 0" in timeStr or " 10:0" in timeStr or " 10:1" in timeStr or " 10:2" in timeStr):
                return "keep sleeping"
            else:
                return "wake up"
            #return "str(colors)"
        except KeyError:
            return request
        except ValueError:
            return request

    conn = sqlite3.connect(clock_db)  #connect to that database (will create if it doesn't already exist)
    c = conn.cursor()  #move cursor into database (allows us to execute commands)
    things = c.execute('''SELECT * FROM senses_table ORDER BY timing DESC;''').fetchall()
    
    light_array = [-x[0] for x in things]
    temp_array = [x[1] for x in things]
    active_array = [x[2] for x in things]
    #sound_array = light_array
    sound_array = [-x[3] for x in things]
    arr = [-x[3] for x in things]
    
    date_array = [x[4] for x in things]
    time_array = [None] * len(date_array)
    
    for i in range(len(time_array)):
        time_array[i] = int(date_array[i][11:13]) + .01*int(date_array[i][14:16])
        
    curDate = date_array[0].split()[0]
    startInd = 0

    ct = 0
    divStr = ""
    plotStr = ""
    
    actives = []
    activeY = []
    date_array.append("end_token yay")
    active_array.append(0)
    
    conn2 = sqlite3.connect(sleep_db)
    c2 = conn.cursor()
    c2.execute('''CREATE TABLE IF NOT EXISTS sleep_table (bed float, rise float, timing timestamp);''')
    c2.execute('''DELETE FROM sleep_table''')

    for i in range(len(date_array)):
        date = date_array[i]
        if active_array[i] == 3:
            actives.append(i)

        if date.split()[0] == curDate:
            #divStr += 'cat'
            bov = 4
            #blah
        else:
            lightStr = "{x: "+str(date_array[startInd:i]) + ", y: "+str(normTo100(light_array[startInd:i]))+", mode: 'lines', line: {color: 'green'}, name: 'Light'}"
            soundStr = "{x: "+str(date_array[startInd:i]) + ", y: "+str(normTo100(sound_array[startInd:i]))+", mode: 'lines', line: {color: 'blue'}, name: 'Sound'}"
            tempStr = "{x: "+str(date_array[startInd:i]) + ", y: "+str(temp_array[startInd:i])+", mode: 'lines', line: {color: 'red'}, name: 'Temperature'}"

            activeStr = "{x:"+str([date_array[ind] for ind in actives])+", y: "+str([temp_array[ind] for ind in actives])+", mode: 'markers', marker: {color: 'rgb(142, 124, 195)', size: 12}, name: 'Woke Times'}"
            divStr += '<div id="plot'+str(ct)+'" style="width:900px;height:400px;"></div>'
            plotStr += '''TESTER = document.getElementById("plot'''+str(ct)+'''");
                    	Plotly.newPlot( TESTER, [''' + soundStr+''', '''+tempStr+''', '''+lightStr+''', '''+activeStr+'''], {
                    	title: "Sleep Analysis '''+curDate+'''"} );'''
            bedTime = 0.0
            riseTime = 1000.0

            for ind in actives:
                if time_array[ind] < 7.00:
                    bedTime = max(time_array[ind], bedTime)
                else:
                    riseTime = min(riseTime, time_array[ind])
            if(curDate == "2020-03-18"):
                bedTime = 2.30
                riseTime = 9.46
            
            if(riseTime < 500):
                foundTime = c2.execute('''SELECT * FROM sleep_table WHERE rise = ?;''', (riseTime,)).fetchone();
                if not foundTime or len(foundTime) == 0:
                    c2.execute('''INSERT into sleep_table VALUES (?,?,?);''', (riseTime, bedTime,date_array[i-1]))
            actives = []
            ct+=1
            curDate = date.split()[0]
            startInd = i
    
    conn2.commit()
    conn2.close()
    
    allTimes = c2.execute('''SELECT * FROM sleep_table ORDER BY timing DESC;''').fetchall();
    for x in allTimes:
        divStr += "<br><p> "+x[2].split()[0]+": "+timeToStr(x[1], x[0])+"</p><br>"
    ret = "";
    for x in things:
        ret += "<p>"+str(x)+"</p><br>"
        
    #return ret
    return ("""<!DOCTYPE html>
<html>
<head>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
	<link rel = 'stylesheet' type='text/css' href='assets/css/todos.css'>
	<link href="https://fonts.googleapis.com/css?family=Roboto:400,500,700" rel="stylesheet">
	<link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.2.0/css/all.css" integrity="sha384-hWVjflwFxL6sNzntih27bfxkr27PmbbK/iSvJ+a4+0owXq79v+lsFkW54bOGbiDQ" crossorigin="anonymous">
	<title>Elevate</title>
</head>
<body>	
    <div id='container'>
		<h1>TO-DO LIST<span id='show'><i class="fas fa-plus"></i></span></h1>
		<input type='text' placeholder='Add New Observation'>
		<ul>
			<li><span class='x'><i class="far fa-trash-alt"></i></span> blah</li>
			<li><span class='x'><i class="far fa-trash-alt"></i></span> glah</li>
			<li><span class='x'><i class="far fa-trash-alt"></i></span> dlah</li>
		</ul>
	</div> """ + 
    divStr + """
	<script type='text/javascript' src='assets/js/lib/jquery-3.3.1.min.js'></script>
	<script type='text/javascript' src='assets/js/todos.js'></script>
    <script> """ + 
    plotStr +"""
</script>
</body>
</html>""")



#+str(allTimes)+" "+ret
    # return 