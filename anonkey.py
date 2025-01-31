import sqlite3
import os
import csv
anonconn = sqlite3.connect('/mnt/c/Users/dtfuentes/anontest/dbanon/ctkDICOM.sql')
phiconn  = sqlite3.connect('/mnt/c/Users/dtfuentes/anontest/dbdirectory/ctkDICOM.sql')

# get data from anonymized db
cursor = anonconn.execute('''
select pt.UID,pt.PatientID,st.StudyInstanceUID,st.StudyDate,se.SeriesInstanceUID,im.SOPInstanceUID, im.Filename as phiUID
                                       from Patients pt join Studies  st on pt.UID= st.PatientsUID 
                                       join Series  se on se.StudyInstanceUID= st.StudyInstanceUID 
                                       join images   im on se.seriesInstanceUID= im.seriesInstanceUID;
''' )
anonymizekey = [ row[:6] + tuple(row[-1].split('/')[-3:]) for row in cursor]
# get phi from anonymized db
cursor = phiconn.execute('''
select pt.UID,pt.PatientID,st.StudyInstanceUID,st.StudyDate,se.SeriesInstanceUID,im.SOPInstanceUID
                                       from Patients pt join Studies  st on pt.UID= st.PatientsUID 
                                       join Series  se on se.StudyInstanceUID= st.StudyInstanceUID 
                                       join images   im on se.seriesInstanceUID= im.seriesInstanceUID ;
''')
phikey = [ row for row in cursor]

try:
   os.remove('./anonymization.sql')
except:
   pass
workconn = sqlite3.connect('./anonymization.sql')
cursor = workconn.cursor()
cursor.execute("CREATE TABLE anonkey(UID,MRN,StudyInstanceUID,StudyDate,SeriesInstanceUID,SOPInstanceUID,phiStudyInstanceUID,phiSeriesInstanceUID,phiSOPInstanceUID)")
cursor.executemany("INSERT INTO anonkey VALUES(?,?,?,?,?,?,?,?,?)", anonymizekey )
workconn.commit()  # Remember to commit the transaction after executing INSERT
cursor.execute("CREATE TABLE phikey(UID,MRN,StudyInstanceUID,StudyDate,SeriesInstanceUID,SOPInstanceUID)")
cursor.executemany("INSERT INTO phikey VALUES(?,?,?,?,?,?)", phikey )
workconn.commit()  # Remember to commit the transaction after executing INSERT
cursor = workconn.execute('''
create table anonymizekey  as
select ak.MRN,ak.StudyInstanceUID,ak.StudyDate,ak.SeriesInstanceUID,ak.SOPInstanceUID,ak.phiStudyInstanceUID,ak.phiSeriesInstanceUID,ak.phiSOPInstanceUID,pk.MRN as phiMRN, pk.StudyDate as phiStudyDate
                 from anonkey ak join phikey  pk  on ak.phiStudyInstanceUID   = pk.StudyInstanceUID
                                                 and ak.phiSeriesInstanceUID  = pk.SeriesInstanceUID
                                                 and ak.phiSOPInstanceUID     = pk.SOPInstanceUID;
''')
cursor = workconn.execute('''
select * from anonymizekey;
''')

with open('anonymizekey.csv', 'w') as csvfile:
  csvwrite = csv.writer(csvfile, delimiter='\t', quotechar='|', quoting=csv.QUOTE_MINIMAL)
  fileHeader =  ['MRN','StudyInstanceUID','StudyDate','SeriesInstanceUID','SOPInstanceUID','phiStudyInstanceUID','phiSeriesInstanceUID','phiSOPInstanceUID','phiMRN','phiStudyDate']
  csvwrite.writerow(fileHeader )
  for row in cursor:
     csvwrite.writerow( row )
