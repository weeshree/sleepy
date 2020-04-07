import time
import requests
import sqlite3
import datetime 
import time
import re
import math
import random

fan_db = "__HOME__/smartClock/fan.db"

def request_handler(request):
    
    if(request['method'] == 'POST'):
        try:
            fanPos = float(request['form']['fanPos'])
            
            conn = sqlite3.connect(fan_db)
            c = conn.cursor()
            c.execute('''CREATE TABLE IF NOT EXISTS fan_table (fanPos float, timing timestamp);''')
            
            c.execute('''INSERT into fan_table VALUES (?,?);''', (fanPos, datetime.datetime.now()- datetime.timedelta(hours = 1)))
            

            conn.commit()
            conn.close()
        
            #return "str(colors)"
        except KeyError:
            return request
        except ValueError:
            return request
    
    elif request['values'] and request['values']['arduino'] and request['values']['arduino'] == 'yes':
        conn = sqlite3.connect(fan_db)
        c = conn.cursor()
        fanPos = c.execute('''SELECT fanPos from fan_table ORDER BY timing DESC;''').fetchone()
        return int(fanPos[0]) #random.randint(0,4)
    return ("""<!DOCTYPE html>
<html>
<body>

<h1>Fan Control</h1>
<form action="/sandbox/sc/snmohan/smartClock/fanController.py" method="post">
  <label for="fanPos">Fan Position (between 0 and 4):</label>
  <select id="fanPos" name="fanPos">
    <option value="0">0</option>
    <option value="1">1</option>
    <option value="2">2</option>
    <option value="3">3</option>
    <option value="4">4</option>
  </select>
  <br>
    <input type="submit">

</form>
</body>
</html>""")

        

