# LPM library Java bindings

## Building

The below is an example of building on Debian 10.  Set the `JAVA_HOME`
according to your JDK version and system.

```shell
$ uname -srv
Linux 4.9.0-9-amd64 #1 SMP Debian 4.9.168-1 (2019-04-12)
$ sudo apt-get install openjdk-8-jre-headless openjdk-8-jdk-headless
...
$ cd liblpm/src/jni
$ JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64 make test
```

This will produce the `liblpm.jar` file.  Use it in your project.
