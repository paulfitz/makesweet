Putting pictures in animated templates.

```
docker pull paulfitz/makesweet
docker run -v $PWD:/share paulfitz/makesweet /reanimator \
  --zip templates/billboard-cityscape.zip \
  --in templates/frog.jpg \
  --gif animation.gif
```
