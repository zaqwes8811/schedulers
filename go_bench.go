
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
    // "github.com/panjf2000/ants"

    // Sys
    // "golang.org/x/sys/unix"
    "runtime"
    // "strconv"
    // "bufio"
    // "os"
    "container/list"
    "sync"

    // 
    "github.com/goinggo/workpool"
)

type GrabbedFrameDescriptor struct {
    frame_idx int
    contents []int

    // 
    // start := time.Now()
    // start := time.Now()
    // ... operation that takes 20 milliseconds ...
    // t := time.Now()
    // elapsed := t.Sub(start)
}

type PerfCouter struct {

}

type MyWork struct {
    frame GrabbedFrameDescriptor
    WP *workpool.WorkPool
    c chan int
}



func (mw *MyWork) DoWork(workRoutine int) {
    // fmt.Printf("%s : %d\n", mw.Name, mw.BirthYear)
    // fmt.Printf("Q:%d R:%d\n", mw.WP.QueuedWork(), mw.WP.ActiveRoutines())

    // Simulate some delay
    // time.Sleep(500 * time.Millisecond)
    mw.c <- mw.frame.frame_idx
}


func main() {
    // Требования для автопилота по камере "Теслы".
    // Фиксированная latence и максимальная fps при заданных ресурсах

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

    // workPool := workpool.New(runtime.NumCPU(), 800)
    workPool := workpool.New(runtime.NumCPU(), 5)
    defer workPool.Shutdown("routine")

    // Main queue - thread-safe should be
    // FIXME: Нужно будет искать по очереди нужный фрейм - потокобезопасно
    // https://yourbasic.org/golang/implement-fifo-queue/

    // https://stackoverflow.com/questions/2818852/is-there-a-queue-implementation
    lock := sync.Mutex{}

    queue := list.New()
    // https://stackoverflow.com/questions/2818852/is-there-a-queue-implementation

    allowed_latency_frames := 5
    
    // Load
    // ticker_next_frame := time.NewTicker(40 * time.Millisecond)
    ticker_next_frame := time.NewTicker(500 * time.Millisecond)
    quit_next_frame := make(chan struct{})
    c := make(chan int)
    go func() {
        global_frame_idx := 0
        for {
           select {
            case <- ticker_next_frame.C:     
                global_frame_idx += 1

                frame := GrabbedFrameDescriptor {
                    frame_idx: global_frame_idx,
                }

                work := MyWork {
                    frame: frame,
                    WP: workPool,
                    c: c,
                }

                // FIXME: push to queue
                // Важно упорядочить
                {
                    lock.Lock()
                    queue.PushBack(frame)
                    for queue.Len() > allowed_latency_frames {
                        e := queue.Front() // First element
                        // FIXME: sent to rnn

                        queue.Remove(e) // Dequeue
                    }
                    lock.Unlock()
                }

                if err := workPool.PostWork("routine", &work); err != nil {
                    fmt.Printf("skip: %s\n", err)
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
                fmt.Printf("PerfCouter:...\n")
            case <- quit_perf:
                ticker_perf.Stop()
                return
            }
        }
     }()

     // Result acceptor - приписывает результаты к фреймам
     go func() {
        for {
            frame_idx := <-c           
            {
                lock.Lock()
                // FIXME: успели-не успели
                // FIXME: нужно искть под локом    
                fmt.Printf("Done frame_idx:%d\n", frame_idx)

                // Put to RNN queue
                lock.Unlock()
            }
        }
     }()

    // Fake Rnn
    go func() {

    }

    time.Sleep(50 * time.Second)
}


