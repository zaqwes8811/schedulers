
// go get -u github.com/panjf2000/ants
// go get -u github.com/goinggo/workpool

// https://www.ardanlabs.com/blog/2013/05/thread-pooling-in-go-programming.html

package main

import (
    "fmt"
    "time"
    // "timer"
    // "log"

    // Mt
    "github.com/panjf2000/ants"

    // Sys
    // "golang.org/x/sys/unix"
    "runtime"
    "strconv"
    "bufio"
    "os"

    // 
    "github.com/goinggo/workpool"
)


type MyWork struct {
    Name string
    BirthYear int
    WP *workpool.WorkPool
}

func (mw *MyWork) DoWork(workRoutine int) {
    fmt.Printf("%s : %d\n", mw.Name, mw.BirthYear)
    fmt.Printf("Q:%d R:%d\n", mw.WP.QueuedWork(), mw.WP.ActiveRoutines())

    // Simulate some delay
    time.Sleep(500 * time.Millisecond)
}


func main() {
    // Требования для автопилота по камере "Теслы".
    // 1. High frame-rate
    // 2. Real-Time/Latency - нужна актуальная информация - входная очередь и скипы фреймов 
    // Время между впавшим в обработу и выпавшим минимальное - lantancey

    // "Реальтаймная/Онлайн система обработки данных с модели камеры"
    // Компоненты - трейдпулы, очереди задачи(потокобезопансые)

    // Эксперимент
    // Меняем количество задейстовванного железа и смотрим на FPS и Latency, 
    // Закон Адмала.

    // 1. Архитектура и инфрастурктуря для запуска
    // 2. в компонентах потюнить параемтер, сколько ядрер, какая длина очередей, количество 
    //   пропусков.

    // FIXME: как по завершения работы дернуть кого-то? чтобы не пуллить. Каналы?

    // https://dev.to/panjf2000/releasing-a-high-performance-goroutine-pool-in-go-n57
    // В опциях нужно выставить MaxBlockingTasks=5 или типа того
    // FIXME: Хз как эти опции применить
    // Worker
    runtime.GOMAXPROCS(runtime.NumCPU())
    pool, _ := ants.NewPool(1)//, ants.WithMaxBlockingTasks(5))
    defer pool.Release()

    // workPool := workpool.New(runtime.NumCPU(), 800)
    workPool := workpool.New(runtime.NumCPU(), 5)
    defer workPool.Shutdown("routine")

    // Load
    ticker_next_frame := time.NewTicker(40 * time.Millisecond)
    quit_next_frame := make(chan struct{})
    go func() {
        for {
           select {
            case <- ticker_next_frame.C:
                // FIXME: ...
                // do stuff
                // Real work multiplexing
                fmt.Println("Next frame")

                i := 0
                work := MyWork {
                    Name: "A" + strconv.Itoa(i),
                    BirthYear: i,
                    WP: workPool,
                }

                if err := workPool.PostWork("routine", &work); err != nil {
                    fmt.Printf("ERROR: %s\n", err)
                    time.Sleep(100 * time.Millisecond)
                }

            case <- quit_next_frame:
                ticker_next_frame.Stop()
                return
            }
        }
     }()

    // 
    ticker_perf := time.NewTicker(1 * time.Second)
    quit_perf := make(chan struct{})
    go func() {
        for {
           select {
            case <- ticker_perf.C:
                // do stuff
                fmt.Println("PerfCouter: ...")
            case <- quit_perf:
                ticker_perf.Stop()
                return
            }
        }
     }()


    time.Sleep(50 * time.Second)
}


func main_new() {
    runtime.GOMAXPROCS(runtime.NumCPU())

    workPool := workpool.New(runtime.NumCPU(), 800)

    shutdown := false // Race Condition, Sorry

    go func() {
        for i := 0; i < 1000; i++ {
            work := MyWork {
                Name: "A" + strconv.Itoa(i),
                BirthYear: i,
                WP: workPool,
            }

            if err := workPool.PostWork("routine", &work); err != nil {
                fmt.Printf("ERROR: %s\n", err)
                time.Sleep(100 * time.Millisecond)
            }

            if shutdown == true {
                return
            }
        }
    }()

    fmt.Println("Hit any key to exit")
    reader := bufio.NewReader(os.Stdin)
    reader.ReadString('\n')

    shutdown = true

    fmt.Println("Shutting Down")
    workPool.Shutdown("routine")
}