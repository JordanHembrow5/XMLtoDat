import matplotlib.pyplot as plt
import sys
import pandas

filename = ''

if len(sys.argv) == 2:
    filename = sys.argv[1]
else:
    print('Error, incorrect call to Python Script')
    exit(1)

x = pandas.read_table(filename, skiprows=[0,1,2,3,4], delimiter='\t', usecols=[0])
y = pandas.read_table(filename, skiprows=[0,1,2,3,4], delimiter='\t', usecols=[1])
plt.plot(x,y,linewidth=0.75,color='b')
plt.xlabel('Position (microns)', fontsize=12)
plt.ylabel('Height (nm)', fontsize=12)
plt.xlim(left=0)

filenamePNG = str.replace(filename,'.dat','.png')
plt.savefig(filenamePNG, transparent=True)
