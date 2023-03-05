package rwe

import (
	"lyfjaberg/xconfig"
	"sync"

	"github.com/benbjohnson/clock"
	pg "github.com/go-pg/pg/v10"
)

var Clock = clock.New()

var (
	once sync.Once
	db_  *pg.DB
	cfg_ *xconfig.Config
)

func GetDB() *pg.DB {
	once.Do(func() {
		db_ = newPostgres(cfg_)
	})
	return db_
}

func GetCfg() *xconfig.Config {
	return cfg_
}

func Init(cfg *xconfig.Config) {
	cfg_ = cfg
	db_ = GetDB()
}

func Exit() {
	db_.Close()
}

func Run(addr string) {

	Router.RunTLS(addr, "/var/www/tracker-key/tracker.cert", "/var/www/tracker-key/tracker.key")
}
