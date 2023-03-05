package main

import (
	"lyfjaberg/rwe"
	"lyfjaberg/xconfig"

	_ "lyfjaberg/tracker"
)

func main() {
	cfg, err := xconfig.LoadConfig()
	if err != nil {
		panic(err)
	}
	rwe.Init(cfg)
	defer rwe.Exit()
	rwe.Run("0.0.0.0:8080")

}
