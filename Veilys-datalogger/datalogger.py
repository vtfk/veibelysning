# Python script som plotter veilysdata fra databasen
# Lisens: CC BY-SA 4.0 - VTFK - 2023

# Importerer nødvendige biblioteker
import mysql.connector
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import datetime
import os
from dotenv import load_dotenv

# Henter ut miljøvariabler fra .env-fil
load_dotenv()

# Kobler til databasen
mydb = mysql.connector.connect(
  host = os.getenv("HOST"),
  user = os.getenv("USER"),
  password = os.getenv("PASSWORD"),
  database = os.getenv("DATABASE")
)

# Henter ut data fra databasen
mycursor = mydb.cursor()
# mycursor.execute("SELECT * FROM veilyslogg")
mycursor.execute("SHOW TABLES")

myresult = mycursor.fetchall()

# Lager lister for å lagre data fra databasen
tid = []
lys = []
manuell = []
dt = []

# Henter ut data fra databasen og klargjør til plotting
for x in myresult:
  tid.append(x[1])
  lys.append(x[3])
  manuell.append(x[4])
  dt.append(datetime.datetime.fromtimestamp(x[1] - 3600*2))

# Plotter veilysdata fra databasen
plt.title('Veilys RV303-PLC01')
plt.xlabel("Dato")
plt.ylabel("Status lys")
plt.yticks([])
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%d.%m.%Y'))
plt.gca().xaxis.set_major_locator(mdates.DayLocator(interval=1))
plt.gca().xaxis.set_minor_locator(mdates.HourLocator(interval=6))
plt.gcf().autofmt_xdate()
plt.grid(axis="x")

plt.scatter(dt, lys)
plt.show()

