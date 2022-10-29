package com.ariasaproject.advancerofrpg;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.Random;

public class ApplicationListener {
    protected final ExecutorService exec;
    
    public ApplicationListener() {
		    exec = Executors.newFixedThreadPool(1, new ThreadFactory() {
		        @Override
		        public Thread newThread(final Runnable r) {
		            Thread thread = new Thread(r, "Background Task");
		            thread.setDaemon(true);
		            return thread;
		        }
		    });
    }
    public void create() {
    }
    public void resize(int width, int height) {
    }

    public void resume() {
    }

    public void render(float delta) {
    }

    public void pause() {
    }
    public void destroy() {
        exec.shutdown();
        try {
            exec.awaitTermination(Long.MAX_VALUE, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            throw new RuntimeException("Couldn't shutdown loading thread", e);
        }
    }
}
