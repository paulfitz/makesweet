Put pictures into animations from the command line.

```
docker run -v $PWD:/share paulfitz/makesweet \
  --zip templates/billboard-cityscape.zip \
  --in images/frog.jpg \
  --gif animation.gif
```

![frog](https://user-images.githubusercontent.com/118367/39386221-9780dec0-4a41-11e8-827d-ec30fea33269.jpg)
![animation](https://user-images.githubusercontent.com/118367/39386216-8f26a80e-4a41-11e8-8ae0-0d44a5a55af1.gif)

```
docker run -v $PWD:/share paulfitz/makesweet \
  --zip templates/heart-locket.zip \
  --start 15 \
  --in images/frog.jpg images/monkey.jpg \
  --gif animation.gif
```

![monkey](https://user-images.githubusercontent.com/118367/44931582-21172300-ad30-11e8-9588-88426fe2671c.jpg)
![frog_monkey_friends](https://user-images.githubusercontent.com/118367/44931507-d8f80080-ad2f-11e8-8804-23cb60b99906.gif)

```
docker run -v $PWD:/share paulfitz/makesweet \
  --zip templates/flag.zip \
  --in images/dolphin.jpg \
  --gif animation.gif
```

![dolphin](https://user-images.githubusercontent.com/118367/44932065-e4e4c200-ad31-11e8-8838-ea6674c50ef5.jpg)
![dolphin_land](https://user-images.githubusercontent.com/118367/44932064-e4e4c200-ad31-11e8-994e-51f68515e505.gif)

Options
-------

You can generate a video output instead of a gif by replacing:
```
  --gif animation.gif
```
with:
```
  --vid animation.mp4
```

See `docker run paulfitz/makesweet -h` for more options.

Source
------

The designs come from https://makesweet.com

