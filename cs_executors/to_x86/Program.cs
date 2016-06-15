using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Threading;

class Executor : IDisposable
{
    private bool disposed;
    private readonly Thread thread;
    private Dispatcher dispatcher;
    private Object thisLock = new Object();

    public Executor()
    {
        thread = new Thread(ThreadMethod);
        thread.Start();
        while (true)
        {
            lock (thisLock)
            {
                if (dispatcher != null)
                    break;
            }
        }
        // object not escape scope while not init
    }

    public void Dispose()
    {
        if (disposed)
            return;

        dispatcher.BeginInvoke(
            new Action(() => Dispatcher.CurrentDispatcher.InvokeShutdown()));
        thread.Join();

        disposed = true;
    }

    public void Post(Action del)
    {
        if (disposed)
            return;

        dispatcher.BeginInvoke(del);
    }

    private void ThreadMethod()
    {
        lock (thisLock)
        {
            dispatcher = Dispatcher.CurrentDispatcher;
        }
        Dispatcher.Run();
    }
}

class Scope : IDisposable
{
    public Scope() 
    {
        if (SynchronizationContext.Current == null)
            SynchronizationContext.SetSynchronizationContext(new SynchronizationContext());

        context = SynchronizationContext.Current;
    }

    public static readonly Executor IO = new Executor();
    public static readonly Executor FS = new Executor();
    static SynchronizationContext context;

    public static void UI_Post(Action del)
    {
        context.Post(s => del(), null);
    }

    public void Dispose()
    {
        IO.Dispose();
        FS.Dispose();
    }
}

class Program
{
    static void Main() 
    {
        using (Scope scope = new Scope()) { 
            Scope.FS.Post(() => {
                Scope.UI_Post(() => done = true);
                Scope.IO.Post(() => 
                {
                    ioCall();
                });
            });

            while (true) {
                if (done) {
                    break;
                }
            }
        }  // dispose
    }

    static bool done = false;
    static void ioCall()
    {
        Console.WriteLine(Thread.CurrentThread.ManagedThreadId);
    }
}
