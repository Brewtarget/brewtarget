# Brewtarget-GuacGUI

A [LinuxServer]() [GuacGui]() image with the development version of [Brewtarget]() installed.

You can read about the base image here.

The version of Brewtarget is from this repository, and may be from the devel branch or one of the other current branches.

## Usage

Here are some example snippets to help you get started creating a container.

### docker

```
docker create \
  --name=brewtarget-guacgui \
  -e PUID=1000 \
  -e PGID=1000 \
  -e TZ=Australia/Melbourne \
  -e GUAC_USER=abc `#optional` \
  -e GUAC_PASS=900150983cd24fb0d6963f7d28e17f72 `#optional` \
  -p 8080:8080 \
  -p 3389:3389 \
  -v </path/to/appdata>:/brewtarget \
  --restart unless-stopped \
  cgspeck/brewtarget-guacgui
```


### docker-compose

Compatible with docker-compose v2 schemas.

```
---
version: "2"
services:
  brewtarget-guacgui:
    image: cgspeck/brewtarget-guacgui
    container_name: brewtarget-guacgui
    environment:
      - PUID=1000
      - PGID=1000
      - TZ=Australia/Melbourne
      - GUAC_USER=abc #optional
      - GUAC_PASS=900150983cd24fb0d6963f7d28e17f72 #optional
    volumes:
      - </path/to/appdata>:/brewtarget
    ports:
      - 8080:8080
      - 3389:3389
    restart: unless-stopped
```

## Parameters

Container images are configured using parameters passed at runtime (such as those above). These parameters are separated by a colon and indicate `<external>:<internal>` respectively. For example, `-p 8080:80` would expose port `80` from inside the container to be accessible from the host's IP on port `8080` outside the container.

| Parameter | Function |
| :----: | --- |
| `-p 8080` | Allows HTTP access to the internal X server. |
| `-p 3389` | Allows RDP access to the internal X server. |
| `-e PUID=1000` | for UserID - see below for explanation |
| `-e PGID=1000` | for GroupID - see below for explanation |
| `-e TZ=Australia/Melbourne` | Specify a timezone to use EG Australia/Melbourne |
| `-e APPNAME=xclock` | Specify the graphical application name shown on RDP access. |
| `-e GUAC_USER=abc` | Specify the username for guacamole's web interface. |
| `-e GUAC_PASS=900150983cd24fb0d6963f7d28e17f72` | Specify the password's md5 hash for guacamole's web interface. |
| `-v /brewtarget` | Contains the Brewtarget user data. |

&nbsp;
## Application Setup

If `GUAC_USER` and `GUAC_PASS` are not set, there is no authentication.
Passwords can be generated via the following:
```
echo -n password | openssl md5
```
```
printf '%s' password | md5sum
```
Please beware this image is not hardened for internet usage. Use
a reverse ssl proxy to increase security.
