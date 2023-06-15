# Python script som plotter veilysdata fra databasen
# Lisens: CC BY-SA 4.0 - VTFK - 2023

import mysql.connector
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import datetime
import os
from dotenv import load_dotenv

load_dotenv()

mydb = mysql.connector.connect(
  host = os.getenv("HOST"),
  user = os.getenv("USER"),
  password = os.getenv("PASSWORD"),
  database = os.getenv("DATABASE")
)

mycursor = mydb.cursor()
mycursor.execute("SELECT * FROM veilys")
myresult = mycursor.fetchall()

tid = []
lys = []
manuell = []
dt = []

for x in myresult:
  tid.append(x[1])
  lys.append(x[3])
  manuell.append(x[4])
  dt.append(datetime.datetime.fromtimestamp(x[1] - 3600*2))

plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%H:%M -  %d.%m.%Y'))
plt.gca().xaxis.set_major_locator(mdates.HourLocator(interval=2))
plt.plot(dt, lys)
# plt.plot(dt, manuell)
plt.gcf().autofmt_xdate()
plt.show()
