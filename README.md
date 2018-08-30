Putting pictures in animated templates.

Input
-----

![frog](https://user-images.githubusercontent.com/118367/39386221-9780dec0-4a41-11e8-827d-ec30fea33269.jpg)

Calculations
------------

```
docker run -v $PWD:/share paulfitz/makesweet /reanimator \
  --zip templates/billboard-cityscape.zip \
  --in templates/frog.jpg \
  --gif animation.gif
```

Output
------

![animation](https://user-images.githubusercontent.com/118367/39386216-8f26a80e-4a41-11e8-8ae0-0d44a5a55af1.gif)

Video format
------------

You can generate a video output instead of a gif by replacing:
```
  --gif animation.gif
```
with:
```
  --vid animation.mp4
```
