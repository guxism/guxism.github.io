package tracker

import (
	"net"
	"time"

	"github.com/google/uuid"
)

type Track struct {
	tableName struct{} `pg:"tracks,alias:t"`

	Id      uuid.UUID `pg:"type:uuid,default:uuid_generate_v4()"`
	IP      net.IP
	FromUrl string
	CreatedAt time.Time
	UserAgent string `header:"User-Agent"`
	Method    string
	Referer   string `header:"Referer"`
	Host      string `header:"Host"`
	Origin    string `header:"Origin"`
}

type Trackin struct {
	FromUrl string `json:"from"`
}
