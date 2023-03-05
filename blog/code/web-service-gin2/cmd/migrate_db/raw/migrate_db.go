package main

import (
	"flag"
	"fmt"

	"lyfjaberg/rwe"
	"lyfjaberg/xconfig"

	migrations "github.com/go-pg/migrations/v8"
	"github.com/sirupsen/logrus"
)

func main() {
	flag.Parse()

	cfg, err := xconfig.LoadConfig()
	if err != nil {
		logrus.Fatal(err)
	}

	rwe.Init(cfg)
	defer rwe.Exit()

	args := flag.Args()
	oldVersion, newVersion, err := migrations.Run(rwe.GetDB(), args...)
	if err != nil {
		logrus.Fatalf("migration %d -> %d failed: %s",
			oldVersion, newVersion, err)
	}

	if newVersion != oldVersion {
		fmt.Printf("migrated from %d to %d\n", oldVersion, newVersion)
	} else {
		fmt.Printf("version is %d\n", oldVersion)
	}
}
