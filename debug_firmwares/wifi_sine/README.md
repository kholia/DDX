Send a sine wave over WiFi ;)

```
nc -lup 12345 | aplay -r 44100 -f S16_LE -t raw -c 1
```
