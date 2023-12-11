
# Importing Image class from PIL module 
from PIL import Image 
 
# Opens a image in RGB mode 
image = Image.open(r"C:\\Users\\woute\\Downloads\\ESP-DASH-master\\ESP-DASH-master\\SindyLoo2.jpg") 
 
image.thumbnail((124, 64), Image.AFFINE)
image.save('C:\\Users\\woute\\Downloads\\ESP-DASH-master\\ESP-DASH-master\\SindyLooHoo.jpg', 'JPEG', quality=88)