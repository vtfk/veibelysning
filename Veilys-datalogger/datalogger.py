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
  user = os.getenv("BRUKER"),
  password = os.getenv("PASSWORD"),
  database = os.getenv("DATABASE")
)

# Henter ut data fra databasen
mycursor = mydb.cursor()
mycursor.execute("SHOW TABLES")
myresult = mycursor.fetchall()

print(myresult)
