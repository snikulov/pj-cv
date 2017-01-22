import csv, sys, urllib
filename = 'pizzas.csv'

with open(filename, 'rb') as f:
    reader = csv.reader(f)
    header = next(reader)
    try:
        for row in reader:
            urllib.urlretrieve ("http://87.255.232.104:7777/debug/image.php?id=%s" % row[0], "%s.jpg" % row[0])
    except csv.Error as e:
        sys.exit('file %s, line %d: %s' % (filename, reader.line_num, e))