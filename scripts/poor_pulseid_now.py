import datetime
#2020-05-08 08:29:52.742737 : 11718049010
reference_date = datetime.datetime(2020, 5, 8, 8, 29, 52)
now = datetime.datetime.utcnow()
delta = (datetime.datetime.utcnow()-reference_date).total_seconds()*1000
print(int(delta/10)+11718049010)
