
var (
	once sync.Once
	db_  *pg.DB
)

func GetDB() *pg.DB {
	once.Do(func() {
		db_ = newPostgres(cfg_)
	})
	return db_
}


class S
{
    public:
        static S& getInstance()
        {
            static S    instance; // Guaranteed to be destroyed.
            return instance;
        }
    private:
        S() {}                    // Constructor? (the {} brackets) are needed here.

        S(S const&);              // Don't Implement
        void operator=(S const&); // Don't implement

    public:
        S(S const&)               = delete;
        void operator=(S const&)  = delete;

};





pub struct Command {
    program: String,
    args: Vec<String>,
    cwd: Option<String>,
}

impl Command {
    pub fn new(program: String) -> Command {
        Command {
            program: program,
            args: Vec::new(),
            cwd: None,
        }
    }
    pub fn arg<'a>(&'a mut self, arg: String) -> &'a mut Command {
        self.args.push(arg);
        self
    }

    pub fn args<'a>(&'a mut self, args: &[String])
                    -> &'a mut Command {
        self.args.push_all(args);
        self
    }

    pub fn cwd<'a>(&'a mut self, dir: String) -> &'a mut Command {
        self.cwd = Some(dir);
        self
    }

    pub fn spawn(&self) -> IoResult<Process> {
    }
}

Command::new("/bin/cat").arg("file.txt").spawn();

let mut cmd = Command::new("/bin/ls");
cmd.arg(".");

if size_sorted {
    cmd.arg("-S");
}

cmd.spawn();


class Abstract {
    public:
      virtual void do() = 0;
};
  
class Factory {
    public:
      Abstract *create();
};


Factory f;
Abstract *a = f.create();
a->do();



class AbstractDispatcher;

class File {
public:
    virtual void Accept(AbstractDispatcher& dispatcher) = 0;
};

class ArchivedFile;
class SplitFile;
class ExtractedFile;

class AbstractDispatcher {
 public:
  virtual void Dispatch(ArchivedFile& file) = 0;
  virtual void Dispatch(SplitFile& file) = 0;
  virtual void Dispatch(ExtractedFile& file) = 0;
};

class ArchivedFile : public File {
 public:
  void Accept(AbstractDispatcher& dispatcher) override {
    dispatcher.Dispatch(*this);
  }
};

class SplitFile : public File {
 public:
  void Accept(AbstractDispatcher& dispatcher) override {
    dispatcher.Dispatch(*this);
  }
};

class ExtractedFile : public File {
 public:
  void Accept(AbstractDispatcher& dispatcher) override {
    dispatcher.Dispatch(*this);
  }
};

class Dispatcher : public AbstractDispatcher {
 public:
  void Dispatch(ArchivedFile&) override {
    std::cout << "dispatching ArchivedFile" << std::endl;
  }

  void Dispatch(SplitFile&) override {
    std::cout << "dispatching SplitFile" << std::endl;
  }

  void Dispatch(ExtractedFile&) override {
    std::cout << "dispatching ExtractedFile" << std::endl;
  }
};

int main() {
  ArchivedFile archived_file;
  SplitFile split_file;
  ExtractedFile extracted_file;

  std::vector<File*> files = {
      &archived_file,
      &split_file,
      &extracted_file,
  };

  Dispatcher dispatcher;
  for (File* file : files) {
    file->Accept(dispatcher);
  }
}


let Subject = {
    _state: 0,
    _observers: [],
    add: function(observer) {
        this._observers.push(observer);
    },
    getState: function() {
        return this._state;
    },
    setState: function(value) {
        this._state = value;
        for (let i = 0; i < this._observers.length; i++)
        {
            this._observers[i].signal(this);
        }
    }
};

let Observer = {
    signal: function(subject) {
        let currentValue = subject.getState();
        console.log(currentValue);
    }
}

Subject.add(Observer);
Subject.setState(10);


