package main

import (
	"log"
	"math/rand"
	"os"
	"strconv"
	"time"
)

func writeFile(num int) {
	f, err := os.OpenFile("/tmp/access.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Fatal(err)
	}
	bs := []byte(strconv.Itoa(num))

	if _, err := f.Write(bs); err != nil {
		log.Fatal(err)
	}
	if err := f.Close(); err != nil {
		log.Fatal(err)
	}
}

func boring() {
	for i := 0; ; i++ {
		writeFile(i)
		time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
	}
}

func main() {
	go boring()
	time.Sleep(2 * time.Second)
}
