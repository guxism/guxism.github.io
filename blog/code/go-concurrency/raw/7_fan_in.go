
package main

import (
	"fmt"
	"math/rand"
	"time"
)

func fanOut(cs ...chan string) {
	n := len(cs)
	go func() {
		for i := 0; ; i++ {
			id := i % n
			msg := fmt.Sprintf("[fan-out %d]: %d", id, i)
			cs[id] <- msg
			time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
		}
	}()
}

func fanIn(cs ...chan string) <-chan string {
	c := make(chan string)
	for i := range cs {
		go func(id int) {
			input := cs[id]
			for {
				c <- <-input
				time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
			}

		}(i)
	}

	return c
}

func boring(id int) chan string {
	c := make(chan string)
	return c
}

func main() {
	var chans [5]chan string
	for i := range chans {
		chans[i] = boring(i)
	}
	fanOut(chans[:]...)
	c := fanIn(chans[:]...)
	for i := 0; i < 20; i++ {
		fmt.Printf("You say: %q\n", <-c)
	}
	fmt.Println("You'r boring, I'am leaving")
}

