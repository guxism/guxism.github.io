package rwe

import (
	"lyfjaberg/xconfig"

	pg "github.com/go-pg/pg/v10"
)

func newPostgres(cfg *xconfig.Config) *pg.DB {
	opt, err := pg.ParseURL(cfg.Postgres_url)
	if err != nil {
		panic(err)
	}

	db := pg.Connect(opt)
	return db
}
