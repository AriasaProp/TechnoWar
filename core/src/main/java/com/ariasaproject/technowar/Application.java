package com.ariasaproject.advancerofrpg;

public interface Application {
    void log(String tag, String message);
    void log(String tag, String message, Throwable exception);
    void error(String tag, String message);
    void error(String tag, String message, Throwable exception);
    void debug(String tag, String message);
    void debug(String tag, String message, Throwable exception);
    void restart();
    void exit();
}
