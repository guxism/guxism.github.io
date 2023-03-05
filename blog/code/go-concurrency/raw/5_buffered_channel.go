package main

import (
	"fmt"
	"math/rand"
	"time"
)

func boring(id int, msg string, ch chan<- string) {
	for i := 0; ; i++ {
		msg := fmt.Sprintf("[%d]: %s %d", id, msg, i)
		ch <- msg
		time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
	}
}

func main() {
	c := make(chan string, 100)
	go boring(1, "boring!", c)
	go boring(2, "boring!", c)
	go boring(3, "boring!", c)
	for i := 0; i < 20; i++ {
		fmt.Printf("You say: %q\n", <-c)
	}
	fmt.Println("You'r boring, I'am leaving")
}
