package lm;

import com.sun.jna.NativeLong;

/**
 * Represents a LedMatrixThread
 */
public class LedMatrixThread {

    public static long DEFAULT_BASE_TIME_NANOS = LmLibrary.DEFAULT_BASE_TIME_NANOS;

    private static LmLibrary INSTANCE = LmLibrary.INSTANCE;
    private final LmLibrary.lmThread lmThread;

    public LedMatrixThread(LedMatrix matrix, long baseTimeNanos) {
        lmThread = INSTANCE.lm_thread_new(matrix.getNative(), new NativeLong(baseTimeNanos));
    }

    public void free() {
        INSTANCE.lm_thread_free(lmThread);
    }

    public void start() {
        INSTANCE.lm_thread_start(lmThread);
    }

    public void pause() {
        INSTANCE.lm_thread_pause(lmThread);
    }

    public void unpause() {
        INSTANCE.lm_thread_unpause(lmThread);
    }

    public void stop() {
        INSTANCE.lm_thread_stop(lmThread);
    }

    public boolean isPaused() {
        return INSTANCE.lm_thread_is_paused(lmThread) > 0;
    }

    public boolean isStopped() {
        return INSTANCE.lm_thread_is_stopped(lmThread) > 0;
    }
}
