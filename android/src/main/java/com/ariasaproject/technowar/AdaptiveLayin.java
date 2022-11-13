package com.ariasaproject.technowar;

public class AdaptiveLayin extends View implements ViewRootImpl.SurfaceChangedCallback {
    final ArrayList<SurfaceHolder.Callback> mCallbacks = new ArrayList<>();
    final int[] mLocation = new int[2];
    final ReentrantLock mSurfaceLock = new ReentrantLock();
    final Surface mSurface = new Surface();
    
    boolean mDrawingStopped = true;
    boolean mDrawFinished = false;
    final Rect mScreenRect = new Rect();
    private final SurfaceSession mSurfaceSession = new SurfaceSession();
    SurfaceControl mSurfaceControl;
    SurfaceControl mBackgroundControl;
    private boolean mDisableBackgroundLayer = false;
    final Object mSurfaceControlLock = new Object();
    final Rect mTmpRect = new Rect();
    Paint mRoundedViewportPaint;
    int mSubLayer = APPLICATION_MEDIA_SUBLAYER;
    boolean mIsCreating = false;
    private final ViewTreeObserver.OnScrollChangedListener mScrollChangedListener = this::updateSurface;
    private final ViewTreeObserver.OnPreDrawListener mDrawListener = () -> {
        // reposition ourselves where the surface is
        mHaveFrame = getWidth() > 0 && getHeight() > 0;
        updateSurface();
        return true;
    };
    boolean mRequestedVisible = false;
    boolean mWindowVisibility = false;
    boolean mLastWindowVisibility = false;
    boolean mViewVisibility = false;
    boolean mWindowStopped = false;
    int mRequestedWidth = -1;
    int mRequestedHeight = -1;
    int mRequestedFormat = PixelFormat.RGB_565;
    boolean mUseAlpha = false;
    float mSurfaceAlpha = 1f;
    boolean mClipSurfaceToBounds;
    int mBackgroundColor = Color.BLACK;
    
    boolean mHaveFrame = false;
    boolean mSurfaceCreated = false;
    long mLastLockTime = 0;
    boolean mVisible = false;
    int mWindowSpaceLeft = -1;
    int mWindowSpaceTop = -1;
    int mSurfaceWidth = -1;
    int mSurfaceHeight = -1;
    float mCornerRadius;
    
    int mFormat = -1;
    final Rect mSurfaceFrame = new Rect();
    int mLastSurfaceWidth = -1, mLastSurfaceHeight = -1;
    int mTransformHint = 0;
    private boolean mGlobalListenersAdded;
    private boolean mAttachedToWindow;
    private int mSurfaceFlags = SurfaceControl.HIDDEN;
    private final SurfaceSyncer mSurfaceSyncer = new SurfaceSyncer();
    private final ArraySet<Integer> mSyncIds = new ArraySet<>();
    private final SurfaceControl.Transaction mRtTransaction = new SurfaceControl.Transaction();
    private final SurfaceControl.Transaction mFrameCallbackTransaction =
            new SurfaceControl.Transaction();
    private int mParentSurfaceSequenceId;
    private RemoteAccessibilityController mRemoteAccessibilityController =
        new RemoteAccessibilityController(this);
    private final Matrix mTmpMatrix = new Matrix();
    SurfaceControlViewHost.SurfacePackage mSurfacePackage;
    private SurfaceControl mBlastSurfaceControl;
    private BLASTBufferQueue mBlastBufferQueue;
    public AdaptiveLayin(Context context) {
        this(context, null);
    }
    public AdaptiveLayin(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }
    public AdaptiveLayin(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }
    public AdaptiveLayin(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        this(context, attrs, defStyleAttr, defStyleRes, false);
    }
    public AdaptiveLayin(Context context, qAttributeSet attrs, int defStyleAttr,
            int defStyleRes, boolean disableBackgroundLayer) {
        super(context, attrs, defStyleAttr, defStyleRes);
        setWillNotDraw(true);
        mDisableBackgroundLayer = disableBackgroundLayer;
    }
    public SurfaceHolder getHolder() {
        return mSurfaceHolder;
    }
    private void updateRequestedVisibility() {
        mRequestedVisible = mViewVisibility && mWindowVisibility && !mWindowStopped;
    }
    private void setWindowStopped(boolean stopped) {
        mWindowStopped = stopped;
        updateRequestedVisibility();
        updateSurface();
    }
    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        getViewRootImpl().addSurfaceChangedCallback(this);
        mWindowStopped = false;
        mViewVisibility = getVisibility() == VISIBLE;
        updateRequestedVisibility();
        mAttachedToWindow = true;
        mParent.requestTransparentRegion(AdaptiveLayin.this);
        if (!mGlobalListenersAdded) {
            ViewTreeObserver observer = getViewTreeObserver();
            observer.addOnScrollChangedListener(mScrollChangedListener);
            observer.addOnPreDrawListener(mDrawListener);
            mGlobalListenersAdded = true;
        }
    }
    @Override
    protected void onWindowVisibilityChanged(int visibility) {
        super.onWindowVisibilityChanged(visibility);
        mWindowVisibility = visibility == VISIBLE;
        updateRequestedVisibility();
        updateSurface();
    }
    @Override
    public void setVisibility(int visibility) {
        super.setVisibility(visibility);
        mViewVisibility = visibility == VISIBLE;
        boolean newRequestedVisible = mWindowVisibility && mViewVisibility && !mWindowStopped;
        if (newRequestedVisible != mRequestedVisible) {
            requestLayout();
        }
        mRequestedVisible = newRequestedVisible;
        updateSurface();
    }
    public void setUseAlpha() {
        if (!mUseAlpha) {
            mUseAlpha = true;
            updateSurfaceAlpha();
        }
    }
    @Override
    public void setAlpha(float alpha) {
        super.setAlpha(alpha);
        updateSurfaceAlpha();
    }
    private float getFixedAlpha() {
        // Compute alpha value to be set on the underlying surface.
        final float alpha = getAlpha();
        return mUseAlpha && (mSubLayer > 0 || alpha == 0f) ? alpha : 1f;
    }
    private void updateSurfaceAlpha() {
        if (!mUseAlpha || !mHaveFrame || mSurfaceControl == null) {
            return;
        }
        final float viewAlpha = getAlpha();
        if (mSubLayer < 0 && 0f < viewAlpha && viewAlpha < 1f) {
            Log.w(TAG, System.identityHashCode(this)
                    + " updateSurfaceAlpha:"
                    + " translucent color is not supported for a surface placed z-below.");
        }
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot == null) {
            return;
        }
        final float alpha = getFixedAlpha();
        if (alpha != mSurfaceAlpha) {
            final Transaction transaction = new Transaction();
            transaction.setAlpha(mSurfaceControl, alpha);
            viewRoot.applyTransactionOnDraw(transaction);
            damageInParent();
            mSurfaceAlpha = alpha;
        }
    }
    private void performDrawFinished() {
        mDrawFinished = true;
        if (mAttachedToWindow) {
            mParent.requestTransparentRegion(AdaptiveLayin.this);
            invalidate();
        }
    }
    @Override
    protected void onDetachedFromWindow() {
        ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot != null) {
            viewRoot.removeSurfaceChangedCallback(this);
        }
        mAttachedToWindow = false;
        if (mGlobalListenersAdded) {
            ViewTreeObserver observer = getViewTreeObserver();
            observer.removeOnScrollChangedListener(mScrollChangedListener);
            observer.removeOnPreDrawListener(mDrawListener);
            mGlobalListenersAdded = false;
        }
        mRequestedVisible = false;
        updateSurface();
        releaseSurfaces(true /* releaseSurfacePackage*/);
        mHaveFrame = false;
        super.onDetachedFromWindow();
    }
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = mRequestedWidth >= 0
                ? resolveSizeAndState(mRequestedWidth, widthMeasureSpec, 0)
                : getDefaultSize(0, widthMeasureSpec);
        int height = mRequestedHeight >= 0
                ? resolveSizeAndState(mRequestedHeight, heightMeasureSpec, 0)
                : getDefaultSize(0, heightMeasureSpec);
        setMeasuredDimension(width, height);
    }
    /** @hide */
    @Override
    protected boolean setFrame(int left, int top, int right, int bottom) {
        boolean result = super.setFrame(left, top, right, bottom);
        updateSurface();
        return result;
    }
    @Override
    public boolean gatherTransparentRegion(Region region) {
        if (isAboveParent() || !mDrawFinished) {
            return super.gatherTransparentRegion(region);
        }
        boolean opaque = true;
        if ((mPrivateFlags & PFLAG_SKIP_DRAW) == 0) {
            // this view draws, remove it from the transparent region
            opaque = super.gatherTransparentRegion(region);
        } else if (region != null) {
            int w = getWidth();
            int h = getHeight();
            if (w>0 && h>0) {
                getLocationInWindow(mLocation);
                // otherwise, punch a hole in the whole hierarchy
                int l = mLocation[0];
                int t = mLocation[1];
                region.op(l, t, l+w, t+h, Region.Op.UNION);
            }
        }
        if (PixelFormat.formatHasAlpha(mRequestedFormat)) {
            opaque = false;
        }
        return opaque;
    }
    @Override
    public void draw(Canvas canvas) {
        if (mDrawFinished && !isAboveParent()) {
            // draw() is not called when SKIP_DRAW is set
            if ((mPrivateFlags & PFLAG_SKIP_DRAW) == 0) {
                // punch a whole in the view-hierarchy below us
                clearAdaptiveLayinPort(canvas);
            }
        }
        super.draw(canvas);
    }
    @Override
    protected void dispatchDraw(Canvas canvas) {
        if (mDrawFinished && !isAboveParent()) {
            // draw() is not called when SKIP_DRAW is set
            if ((mPrivateFlags & PFLAG_SKIP_DRAW) == PFLAG_SKIP_DRAW) {
                // punch a whole in the view-hierarchy below us
                clearAdaptiveLayinPort(canvas);
            }
        }
        super.dispatchDraw(canvas);
    }
    public void setEnableSurfaceClipping(boolean enabled) {
        mClipSurfaceToBounds = enabled;
        invalidate();
    }
    @Override
    public void setClipBounds(Rect clipBounds) {
        super.setClipBounds(clipBounds);
        if (!mClipSurfaceToBounds || mSurfaceControl == null) {
            return;
        }
        // When cornerRadius is non-zero, a draw() is required to update
        // the viewport (rounding the corners of the clipBounds).
        if (mCornerRadius > 0f && !isAboveParent()) {
            invalidate();
        }
        if (mClipBounds != null) {
            mTmpRect.set(mClipBounds);
        } else {
            mTmpRect.set(0, 0, mSurfaceWidth, mSurfaceHeight);
        }
        final Transaction transaction = new Transaction();
        transaction.setWindowCrop(mSurfaceControl, mTmpRect);
        applyTransactionOnVriDraw(transaction);
        invalidate();
    }
    private void clearAdaptiveLayinPort(Canvas canvas) {
        if (mCornerRadius > 0f) {
            canvas.getClipBounds(mTmpRect);
            if (mClipSurfaceToBounds && mClipBounds != null) {
                mTmpRect.intersect(mClipBounds);
            }
            canvas.punchHole(
                    mTmpRect.left,
                    mTmpRect.top,
                    mTmpRect.right,
                    mTmpRect.bottom,
                    mCornerRadius,
                    mCornerRadius
            );
        } else {
            canvas.punchHole(0f, 0f, getWidth(), getHeight(), 0f, 0f);
        }
    }
    public void setCornerRadius(float cornerRadius) {
        mCornerRadius = cornerRadius;
        if (mCornerRadius > 0f && mRoundedViewportPaint == null) {
            mRoundedViewportPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mRoundedViewportPaint.setBlendMode(BlendMode.CLEAR);
            mRoundedViewportPaint.setColor(0);
        }
        invalidate();
    }
    public float getCornerRadius() {
        return mCornerRadius;
    }
    public void setZOrderMediaOverlay(boolean isMediaOverlay) {
        mSubLayer = isMediaOverlay
            ? APPLICATION_MEDIA_OVERLAY_SUBLAYER : APPLICATION_MEDIA_SUBLAYER;
    }
    public void setZOrderOnTop(boolean onTop) {
        // In R and above we allow dynamic layer changes.
        final boolean allowDynamicChange = getContext().getApplicationInfo().targetSdkVersion
                > Build.VERSION_CODES.Q;
        setZOrderedOnTop(onTop, allowDynamicChange);
    }
    public boolean isZOrderedOnTop() {
        return mSubLayer > 0;
    }
    public boolean setZOrderedOnTop(boolean onTop, boolean allowDynamicChange) {
        final int subLayer;
        if (onTop) {
            subLayer = APPLICATION_PANEL_SUBLAYER;
        } else {
            subLayer = APPLICATION_MEDIA_SUBLAYER;
        }
        if (mSubLayer == subLayer) {
            return false;
        }
        mSubLayer = subLayer;
        if (!allowDynamicChange) {
            return false;
        }
        if (mSurfaceControl == null) {
            return true;
        }
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot == null) {
            return true;
        }
        final Transaction transaction = new SurfaceControl.Transaction();
        updateRelativeZ(transaction);
        viewRoot.applyTransactionOnDraw(transaction);
        invalidate();
        return true;
    }
    public void setSecure(boolean isSecure) {
        if (isSecure) {
            mSurfaceFlags |= SurfaceControl.SECURE;
        } else {
            mSurfaceFlags &= ~SurfaceControl.SECURE;
        }
    }
    private void updateOpaqueFlag() {
        if (!PixelFormat.formatHasAlpha(mRequestedFormat)) {
            mSurfaceFlags |= SurfaceControl.OPAQUE;
        } else {
            mSurfaceFlags &= ~SurfaceControl.OPAQUE;
        }
    }
    private void updateBackgroundVisibility(Transaction t) {
        if (mBackgroundControl == null) {
            return;
        }
        if ((mSubLayer < 0) && ((mSurfaceFlags & SurfaceControl.OPAQUE) != 0)
                && !mDisableBackgroundLayer) {
            t.show(mBackgroundControl);
        } else {
            t.hide(mBackgroundControl);
        }
    }
    private Transaction updateBackgroundColor(Transaction t) {
        final float[] colorComponents = new float[] { Color.red(mBackgroundColor) / 255.f,
                Color.green(mBackgroundColor) / 255.f, Color.blue(mBackgroundColor) / 255.f };
        t.setColor(mBackgroundControl, colorComponents);
        return t;
    }
    private void releaseSurfaces(boolean releaseSurfacePackage) {
        mSurfaceAlpha = 1f;
        mSurface.destroy();
        synchronized (mSurfaceControlLock) {
            if (mBlastBufferQueue != null) {
                mBlastBufferQueue.destroy();
                mBlastBufferQueue = null;
            }
            final Transaction transaction = new Transaction();
            if (mSurfaceControl != null) {
                transaction.remove(mSurfaceControl);
                mSurfaceControl = null;
            }
            if (mBackgroundControl != null) {
                transaction.remove(mBackgroundControl);
                mBackgroundControl = null;
            }
            if (mBlastSurfaceControl != null) {
                transaction.remove(mBlastSurfaceControl);
                mBlastSurfaceControl = null;
            }
            if (releaseSurfacePackage && mSurfacePackage != null) {
                mSurfacePackage.release();
                mSurfacePackage = null;
            }
            applyTransactionOnVriDraw(transaction);
        }
    }
    private void replacePositionUpdateListener(int surfaceWidth, int surfaceHeight) {
        if (mPositionListener != null) {
            mRenderNode.removePositionUpdateListener(mPositionListener);
        }
        mPositionListener = new AdaptiveLayinPositionUpdateListener(surfaceWidth, surfaceHeight);
        mRenderNode.addPositionUpdateListener(mPositionListener);
    }
    private boolean performSurfaceTransaction(ViewRootImpl viewRoot, Translator translator,
            boolean creating, boolean sizeChanged, boolean hintChanged,
            Transaction surfaceUpdateTransaction) {
        boolean realSizeChanged = false;
        mDrawingStopped = !mVisible;
        if (creating) {
            updateRelativeZ(surfaceUpdateTransaction);
            if (mSurfacePackage != null) {
                reparentSurfacePackage(surfaceUpdateTransaction, mSurfacePackage);
            }
        }
        mParentSurfaceSequenceId = viewRoot.getSurfaceSequenceId();
        if (mViewVisibility) {
            surfaceUpdateTransaction.show(mSurfaceControl);
        } else {
            surfaceUpdateTransaction.hide(mSurfaceControl);
        }
        updateBackgroundVisibility(surfaceUpdateTransaction);
        updateBackgroundColor(surfaceUpdateTransaction);
        if (mUseAlpha) {
            float alpha = getFixedAlpha();
            surfaceUpdateTransaction.setAlpha(mSurfaceControl, alpha);
            mSurfaceAlpha = alpha;
        }
        surfaceUpdateTransaction.setCornerRadius(mSurfaceControl, mCornerRadius);
        if ((sizeChanged || hintChanged) && !creating) {
            setBufferSize(surfaceUpdateTransaction);
        }
        if (sizeChanged || creating || !isHardwareAccelerated()) {
            // Set a window crop when creating the surface or changing its size to
            // crop the buffer to the surface size since the buffer producer may
            // use SCALING_MODE_SCALE and submit a larger size than the surface
            // size.
            if (mClipSurfaceToBounds && mClipBounds != null) {
                surfaceUpdateTransaction.setWindowCrop(mSurfaceControl, mClipBounds);
            } else {
                surfaceUpdateTransaction.setWindowCrop(mSurfaceControl, mSurfaceWidth,
                        mSurfaceHeight);
            }
            surfaceUpdateTransaction.setDesintationFrame(mBlastSurfaceControl, mSurfaceWidth,
                        mSurfaceHeight);
            if (isHardwareAccelerated()) {
                // This will consume the passed in transaction and the transaction will be
                // applied on a render worker thread.
                replacePositionUpdateListener(mSurfaceWidth, mSurfaceHeight);
            } else {
                onSetSurfacePositionAndScale(surfaceUpdateTransaction, mSurfaceControl,
                        mScreenRect.left /*positionLeft*/,
                        mScreenRect.top /*positionTop*/,
                        mScreenRect.width() / (float) mSurfaceWidth /*postScaleX*/,
                        mScreenRect.height() / (float) mSurfaceHeight /*postScaleY*/);
            }
        }
        applyTransactionOnVriDraw(surfaceUpdateTransaction);
        updateEmbeddedAccessibilityMatrix(false);
         mSurfaceFrame.left = 0;
        mSurfaceFrame.top = 0;
        if (translator == null) {
            mSurfaceFrame.right = mSurfaceWidth;
            mSurfaceFrame.bottom = mSurfaceHeight;
        } else {
            float appInvertedScale = translator.applicationInvertedScale;
            mSurfaceFrame.right = (int) (mSurfaceWidth * appInvertedScale + 0.5f);
            mSurfaceFrame.bottom = (int) (mSurfaceHeight * appInvertedScale + 0.5f);
        }
        final int surfaceWidth = mSurfaceFrame.right;
        final int surfaceHeight = mSurfaceFrame.bottom;
        realSizeChanged = mLastSurfaceWidth != surfaceWidth
                || mLastSurfaceHeight != surfaceHeight;
        mLastSurfaceWidth = surfaceWidth;
        mLastSurfaceHeight = surfaceHeight;
        return realSizeChanged;
    }
    protected void updateSurface() {
        if (!mHaveFrame) {
            return;
        }
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot == null) {
            return;
        }
        if (viewRoot.mSurface == null || !viewRoot.mSurface.isValid()) {
            notifySurfaceDestroyed();
            releaseSurfaces(false);
            return;
        }
        final Translator translator = viewRoot.mTranslator;
        if (translator != null) {
            mSurface.setCompatibilityTranslator(translator);
        }
        int myWidth = mRequestedWidth;
        if (myWidth <= 0) myWidth = getWidth();
        int myHeight = mRequestedHeight;
        if (myHeight <= 0) myHeight = getHeight();
        final float alpha = getFixedAlpha();
        final boolean formatChanged = mFormat != mRequestedFormat;
        final boolean visibleChanged = mVisible != mRequestedVisible;
        final boolean alphaChanged = mSurfaceAlpha != alpha;
        final boolean creating = (mSurfaceControl == null || formatChanged || visibleChanged)
                && mRequestedVisible;
        final boolean sizeChanged = mSurfaceWidth != myWidth || mSurfaceHeight != myHeight;
        final boolean windowVisibleChanged = mWindowVisibility != mLastWindowVisibility;
        getLocationInSurface(mLocation);
        final boolean positionChanged = mWindowSpaceLeft != mLocation[0]
            || mWindowSpaceTop != mLocation[1];
        final boolean layoutSizeChanged = getWidth() != mScreenRect.width()
            || getHeight() != mScreenRect.height();
        final boolean hintChanged = (viewRoot.getBufferTransformHint() != mTransformHint)
                && mRequestedVisible;
        if (creating || formatChanged || sizeChanged || visibleChanged ||
                (mUseAlpha && alphaChanged) || windowVisibleChanged ||
                positionChanged || layoutSizeChanged || hintChanged) {
            getLocationInWindow(mLocation);
            try {
                mVisible = mRequestedVisible;
                mWindowSpaceLeft = mLocation[0];
                mWindowSpaceTop = mLocation[1];
                mSurfaceWidth = myWidth;
                mSurfaceHeight = myHeight;
                mFormat = mRequestedFormat;
                mLastWindowVisibility = mWindowVisibility;
                mTransformHint = viewRoot.getBufferTransformHint();
                mScreenRect.left = mWindowSpaceLeft;
                mScreenRect.top = mWindowSpaceTop;
                mScreenRect.right = mWindowSpaceLeft + getWidth();
                mScreenRect.bottom = mWindowSpaceTop + getHeight();
                if (translator != null) {
                    translator.translateRectInAppWindowToScreen(mScreenRect);
                }
                final Rect surfaceInsets = viewRoot.mWindowAttributes.surfaceInsets;
                mScreenRect.offset(surfaceInsets.left, surfaceInsets.top);
                // Collect all geometry changes and apply these changes on the RenderThread worker
                // via the RenderNode.PositionUpdateListener.
                final Transaction surfaceUpdateTransaction = new Transaction();
                if (creating) {
                    updateOpaqueFlag();
                    final String name = "AdaptiveLayin[" + viewRoot.getTitle().toString() + "]";
                    createBlastSurfaceControls(viewRoot, name, surfaceUpdateTransaction);
                } else if (mSurfaceControl == null) {
                    return;
                }
                final boolean redrawNeeded = sizeChanged || creating || hintChanged
                        || (mVisible && !mDrawFinished);
                boolean shouldSyncBuffer =
                        redrawNeeded && viewRoot.wasRelayoutRequested() && viewRoot.isInLocalSync();
                SyncBufferTransactionCallback syncBufferTransactionCallback = null;
                if (shouldSyncBuffer) {
                    syncBufferTransactionCallback = new SyncBufferTransactionCallback();
                    mBlastBufferQueue.syncNextTransaction(
                            false /* acquireSingleBuffer */,
                            syncBufferTransactionCallback::onTransactionReady);
                }
                final boolean realSizeChanged = performSurfaceTransaction(viewRoot,
                        translator, creating, sizeChanged, hintChanged, surfaceUpdateTransaction);
                try {
                    SurfaceHolder.Callback[] callbacks = null;
                    final boolean surfaceChanged = creating;
                    if (mSurfaceCreated && (surfaceChanged || (!mVisible && visibleChanged))) {
                        mSurfaceCreated = false;
                        notifySurfaceDestroyed();
                    }
                    copySurface(creating, sizeChanged);
                    if (mVisible && mSurface.isValid()) {
                        if (!mSurfaceCreated && (surfaceChanged || visibleChanged)) {
                            mSurfaceCreated = true;
                            mIsCreating = true;
                            callbacks = getSurfaceCallbacks();
                            for (SurfaceHolder.Callback c : callbacks) {
                                c.surfaceCreated(mSurfaceHolder);
                            }
                        }
                        if (creating || formatChanged || sizeChanged || hintChanged || visibleChanged || realSizeChanged) {
                            if (callbacks == null) {
                                callbacks = getSurfaceCallbacks();
                            }
                            for (SurfaceHolder.Callback c : callbacks) {
                                c.surfaceChanged(mSurfaceHolder, mFormat, myWidth, myHeight);
                            }
                        }
                        if (redrawNeeded) {
                            if (callbacks == null) {
                                callbacks = getSurfaceCallbacks();
                            }
                            if (shouldSyncBuffer) {
                                handleSyncBufferCallback(callbacks, syncBufferTransactionCallback);
                            } else {
                                handleSyncNoBuffer(callbacks);
                            }
                        }
                    }
                } finally {
                    mIsCreating = false;
                    if (mSurfaceControl != null && !mSurfaceCreated) {
                        releaseSurfaces(false /* releaseSurfacePackage*/);
                    }
                }
            } catch (Exception ex) {
                Log.e(TAG, "Exception configuring surface", ex);
            }
        }
    }
    /**
     * If SV is trying to be part of the VRI sync, we need to add SV to the VRI sync before
     * invoking the redrawNeeded call to the owner. This is to ensure we can set up the SV in
     * the sync before the SV owner knows it needs to draw a new frame.
     * Once the redrawNeeded callback is invoked, we can stop the continuous sync transaction
     * call which will invoke the syncTransaction callback that contains the buffer. The
     * code waits until we can retrieve the transaction that contains the buffer before
     * notifying the syncer that the buffer is ready.
     */
    private void handleSyncBufferCallback(SurfaceHolder.Callback[] callbacks,
            SyncBufferTransactionCallback syncBufferTransactionCallback) {
        getViewRootImpl().addToSync(syncBufferCallback ->
                redrawNeededAsync(callbacks, () -> {
                    Transaction t = null;
                    if (mBlastBufferQueue != null) {
                        mBlastBufferQueue.stopContinuousSyncTransaction();
                        t = syncBufferTransactionCallback.waitForTransaction();
                    }
                    syncBufferCallback.onBufferReady(t);
                    onDrawFinished();
                }));
    }
    private void handleSyncNoBuffer(SurfaceHolder.Callback[] callbacks) {
        final int syncId = mSurfaceSyncer.setupSync(this::onDrawFinished);
        mSurfaceSyncer.addToSync(syncId, syncBufferCallback -> redrawNeededAsync(callbacks,
                () -> {
                    syncBufferCallback.onBufferReady(null);
                    synchronized (mSyncIds) {
                        mSyncIds.remove(syncId);
                    }
                }));
        mSurfaceSyncer.markSyncReady(syncId);
        synchronized (mSyncIds) {
            mSyncIds.add(syncId);
        }
    }
    private void redrawNeededAsync(SurfaceHolder.Callback[] callbacks,
            Runnable callbacksCollected) {
        SurfaceCallbackHelper sch = new SurfaceCallbackHelper(callbacksCollected);
        sch.dispatchSurfaceRedrawNeededAsync(mSurfaceHolder, callbacks);
    }
    @Override
    public void surfaceSyncStarted() {
        ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot != null) {
            synchronized (mSyncIds) {
                for (int syncId : mSyncIds) {
                    viewRoot.mergeSync(syncId, mSurfaceSyncer);
                }
            }
        }
    }
    private static class SyncBufferTransactionCallback {
        private final CountDownLatch mCountDownLatch = new CountDownLatch(1);
        private Transaction mTransaction;
        Transaction waitForTransaction() {
            try {
                mCountDownLatch.await();
            } catch (InterruptedException e) {
            }
            return mTransaction;
        }
        void onTransactionReady(Transaction t) {
            mTransaction = t;
            mCountDownLatch.countDown();
        }
    }
    private void copySurface(boolean surfaceControlCreated, boolean bufferSizeChanged) {
        boolean needsWorkaround = bufferSizeChanged && getContext().getApplicationInfo().targetSdkVersion < Build.VERSION_CODES.O;
       if (!surfaceControlCreated && !needsWorkaround) {
           return;
       }
       mSurfaceLock.lock();
       try {
           if (surfaceControlCreated) {
               mSurface.copyFrom(mBlastBufferQueue);
           }
           if (needsWorkaround) {
               if (mBlastBufferQueue != null) {
                   mSurface.transferFrom(mBlastBufferQueue.createSurfaceWithHandle());
               }
           }
       } finally {
           mSurfaceLock.unlock();
       }
    }
    private void setBufferSize(Transaction transaction) {
        mBlastSurfaceControl.setTransformHint(mTransformHint);
        if (mBlastBufferQueue != null) {
            mBlastBufferQueue.update(mBlastSurfaceControl, mSurfaceWidth, mSurfaceHeight,
                        mFormat);
        }
    } 
    
    private void createBlastSurfaceControls(ViewRootImpl viewRoot, String name,
            Transaction surfaceUpdateTransaction) {
        if (mSurfaceControl == null) {
            mSurfaceControl = new SurfaceControl.Builder(mSurfaceSession)
                    .setName(name)
                    .setLocalOwnerView(this)
                    .setParent(viewRoot.getBoundsLayer())
                    .setCallsite("AdaptiveLayin.updateSurface")
                    .setContainerLayer()
                    .build();
        }
        if (mBlastSurfaceControl == null) {
            mBlastSurfaceControl = new SurfaceControl.Builder(mSurfaceSession)
                    .setName(name + "(BLAST)")
                    .setLocalOwnerView(this)
                    .setParent(mSurfaceControl)
                    .setFlags(mSurfaceFlags)
                    .setHidden(false)
                    .setBLASTLayer()
                    .setCallsite("AdaptiveLayin.updateSurface")
                    .build();
        } else {
            // update blast layer
            surfaceUpdateTransaction
                    .setOpaque(mBlastSurfaceControl, (mSurfaceFlags & SurfaceControl.OPAQUE) != 0)
                    .setSecure(mBlastSurfaceControl, (mSurfaceFlags & SurfaceControl.SECURE) != 0)
                    .show(mBlastSurfaceControl);
        }
        if (mBackgroundControl == null) {
            mBackgroundControl = new SurfaceControl.Builder(mSurfaceSession)
                    .setName("Background for " + name)
                    .setLocalOwnerView(this)
                    .setOpaque(true)
                    .setColorLayer()
                    .setParent(mSurfaceControl)
                    .setCallsite("AdaptiveLayin.updateSurface")
                    .build();
        }
        // Always recreate the IGBP for compatibility. This can be optimized in the future but
        // the behavior change will need to be gated by SDK version.
        if (mBlastBufferQueue != null) {
            mBlastBufferQueue.destroy();
        }
        mTransformHint = viewRoot.getBufferTransformHint();
        mBlastSurfaceControl.setTransformHint(mTransformHint);
        mBlastBufferQueue = new BLASTBufferQueue(name, false /* updateDestinationFrame */);
        mBlastBufferQueue.update(mBlastSurfaceControl, mSurfaceWidth, mSurfaceHeight, mFormat);
        mBlastBufferQueue.setTransactionHangCallback(ViewRootImpl.sTransactionHangCallback);
    }
    private void onDrawFinished() {
        runOnUiThread(this::performDrawFinished);
    }
    protected void onSetSurfacePositionAndScale(Transaction transaction,
            SurfaceControl surface, int positionLeft, int positionTop,
            float postScaleX, float postScaleY) {
        transaction.setPosition(surface, positionLeft, positionTop);
        transaction.setMatrix(surface, postScaleX /*dsdx*/, 0f /*dtdx*/,
                0f /*dtdy*/, postScaleY /*dsdy*/);
  }
    public void requestUpdateSurfacePositionAndScale() {
        if (mSurfaceControl == null) {
            return;
        }
        final Transaction transaction = new Transaction();
        onSetSurfacePositionAndScale(transaction, mSurfaceControl,
                mScreenRect.left, /*positionLeft*/
                mScreenRect.top/*positionTop*/ ,
                mScreenRect.width() / (float) mSurfaceWidth /*postScaleX*/,
                mScreenRect.height() / (float) mSurfaceHeight /*postScaleY*/);
        applyTransactionOnVriDraw(transaction);
        invalidate();
    }
    public Rect getSurfaceRenderPosition() {
        return mRTLastReportedPosition;
    }
    private void applyOrMergeTransaction(Transaction t, long frameNumber) {
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot != null) {
            // If we are using BLAST, merge the transaction with the viewroot buffer transaction.
            viewRoot.mergeWithNextTransaction(t, frameNumber);
        } else {
            t.apply();
        }
    }
    private final Rect mRTLastReportedPosition = new Rect();
    private final Point mRTLastReportedSurfaceSize = new Point();
    private class AdaptiveLayinPositionUpdateListener implements RenderNode.PositionUpdateListener {
        private final int mRtSurfaceWidth;
        private final int mRtSurfaceHeight;
        private boolean mRtFirst = true;
        private final SurfaceControl.Transaction mPositionChangedTransaction =
                new SurfaceControl.Transaction();
        AdaptiveLayinPositionUpdateListener(int surfaceWidth, int surfaceHeight) {
            mRtSurfaceWidth = surfaceWidth;
            mRtSurfaceHeight = surfaceHeight;
        }
        @Override
        public void positionChanged(long frameNumber, int left, int top, int right, int bottom) {
            if (!mRtFirst && (mRTLastReportedPosition.left == left
                    && mRTLastReportedPosition.top == top
                    && mRTLastReportedPosition.right == right
                    && mRTLastReportedPosition.bottom == bottom
                    && mRTLastReportedSurfaceSize.x == mRtSurfaceWidth
                    && mRTLastReportedSurfaceSize.y == mRtSurfaceHeight)) {
                return;
            }
            mRtFirst = false;
            try {
                synchronized (mSurfaceControlLock) {
                    if (mSurfaceControl == null) return;
                    mRTLastReportedPosition.set(left, top, right, bottom);
                    mRTLastReportedSurfaceSize.set(mRtSurfaceWidth, mRtSurfaceHeight);
                    onSetSurfacePositionAndScale(mPositionChangedTransaction, mSurfaceControl,
                            mRTLastReportedPosition.left /*positionLeft*/,
                            mRTLastReportedPosition.top /*positionTop*/,
                            mRTLastReportedPosition.width()
                                    / (float) mRtSurfaceWidth /*postScaleX*/,
                            mRTLastReportedPosition.height()
                                    / (float) mRtSurfaceHeight /*postScaleY*/);
                    if (mViewVisibility) {
                        // b/131239825
                        mPositionChangedTransaction.show(mSurfaceControl);
                    }
                }
                applyOrMergeTransaction(mPositionChangedTransaction, frameNumber);
            } catch (Exception ex) {
                Log.e(TAG, "Exception from repositionChild", ex);
            }
        }
        @Override
        public void applyStretch(long frameNumber, float width, float height,
                float vecX, float vecY, float maxStretchX, float maxStretchY,
                float childRelativeLeft, float childRelativeTop, float childRelativeRight,
                float childRelativeBottom) {
            mRtTransaction.setStretchEffect(mSurfaceControl, width, height, vecX, vecY,
                    maxStretchX, maxStretchY, childRelativeLeft, childRelativeTop,
                    childRelativeRight, childRelativeBottom);
            applyOrMergeTransaction(mRtTransaction, frameNumber);
        }
        @Override
        public void positionLost(long frameNumber) {
            mRTLastReportedPosition.setEmpty();
            mRTLastReportedSurfaceSize.set(-1, -1);
            // positionLost can be called while UI thread is un-paused.
            synchronized (mSurfaceControlLock) {
                if (mSurfaceControl == null) return;
                // b/131239825
                mRtTransaction.hide(mSurfaceControl);
                applyOrMergeTransaction(mRtTransaction, frameNumber);
            }
        }
    }
    private AdaptiveLayinPositionUpdateListener mPositionListener = null;
    private SurfaceHolder.Callback[] getSurfaceCallbacks() {
        SurfaceHolder.Callback[] callbacks;
        synchronized (mCallbacks) {
            callbacks = new SurfaceHolder.Callback[mCallbacks.size()];
            mCallbacks.toArray(callbacks);
        }
        return callbacks;
    }
    private void runOnUiThread(Runnable runnable) {
        Handler handler = getHandler();
        if (handler != null && handler.getLooper() != Looper.myLooper()) {
            handler.post(runnable);
        } else {
            runnable.run();
        }
    }
    public boolean isFixedSize() {
        return (mRequestedWidth != -1 || mRequestedHeight != -1);
    }
    private boolean isAboveParent() {
        return mSubLayer >= 0;
    }
    public void setResizeBackgroundColor(int bgColor) {
        final SurfaceControl.Transaction transaction = new SurfaceControl.Transaction();
        setResizeBackgroundColor(transaction, bgColor);
        applyTransactionOnVriDraw(transaction);
        invalidate();
    }
    public void setResizeBackgroundColor(SurfaceControl.Transaction t, int bgColor) {
        if (mBackgroundControl == null) {
            return;
        }
        mBackgroundColor = bgColor;
        updateBackgroundColor(t);
    }
    
    private final SurfaceHolder mSurfaceHolder = new SurfaceHolder() {
        private static final String LOG_TAG = "SurfaceHolder";
        @Override
        public boolean isCreating() {
            return mIsCreating;
        }
        @Override
        public void addCallback(Callback callback) {
            synchronized (mCallbacks) {
                // This is a linear search, but in practice we'll
                // have only a couple callbacks, so it doesn't matter.
                if (!mCallbacks.contains(callback)) {
                    mCallbacks.add(callback);
                }
            }
        }
        @Override
        public void removeCallback(Callback callback) {
            synchronized (mCallbacks) {
                mCallbacks.remove(callback);
            }
        }
        @Override
        public void setFixedSize(int width, int height) {
            if (mRequestedWidth != width || mRequestedHeight != height) {
                mRequestedWidth = width;
                mRequestedHeight = height;
                requestLayout();
            }
        }
        @Override
        public void setSizeFromLayout() {
            if (mRequestedWidth != -1 || mRequestedHeight != -1) {
                mRequestedWidth = mRequestedHeight = -1;
                requestLayout();
            }
        }
        @Override
        public void setFormat(int format) {
            if (format == PixelFormat.OPAQUE)
                format = PixelFormat.RGB_565;
            mRequestedFormat = format;
            if (mSurfaceControl != null) {
                updateSurface();
            }
        }
        @Override
        public void setKeepScreenOn(boolean screenOn) {
            runOnUiThread(() -> AdaptiveLayin.this.setKeepScreenOn(screenOn));
        }
        @Override
        public Canvas lockCanvas() {
            return internalLockCanvas(null, false);
        }
        @Override
        public Canvas lockCanvas(Rect inOutDirty) {
            return internalLockCanvas(inOutDirty, false);
        }
        @Override
        public Canvas lockHardwareCanvas() {
            return internalLockCanvas(null, true);
        }
        private Canvas internalLockCanvas(Rect dirty, boolean hardware) {
            mSurfaceLock.lock();
            Canvas c = null;
            if (!mDrawingStopped && mSurfaceControl != null) {
                try {
                    if (hardware) {
                        c = mSurface.lockHardwareCanvas();
                    } else {
                        c = mSurface.lockCanvas(dirty);
                    }
                } catch (Exception e) {
                    Log.e(LOG_TAG, "Exception locking surface", e);
                }
            }
            if (c != null) {
                mLastLockTime = SystemClock.uptimeMillis();
                return c;
            }
            // If the Surface is not ready to be drawn, then return null,
            // but throttle calls to this function so it isn't called more
            // than every 100ms.
            long now = SystemClock.uptimeMillis();
            long nextTime = mLastLockTime + 100;
            if (nextTime > now) {
                try {
                    Thread.sleep(nextTime-now);
                } catch (InterruptedException e) {
                }
                now = SystemClock.uptimeMillis();
            }
            mLastLockTime = now;
            mSurfaceLock.unlock();
            return null;
        }
        /**
         * Posts the new contents of the {@link Canvas} to the surface and
         * releases the {@link Canvas}.
         *
         * @param canvas The canvas previously obtained from {@link #lockCanvas}.
         */
        @Override
        public void unlockCanvasAndPost(Canvas canvas) {
            try {
                mSurface.unlockCanvasAndPost(canvas);
            } finally {
                mSurfaceLock.unlock();
            }
        }
        @Override
        public Surface getSurface() {
            return mSurface;
        }
        @Override
        public Rect getSurfaceFrame() {
            return mSurfaceFrame;
        }
    };
    
    public SurfaceControl getSurfaceControl() {
        return mSurfaceControl;
    }
    public IBinder getHostToken() {
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot == null) {
            return null;
        }
        return viewRoot.getInputToken();
    } 
    @Override
    public void surfaceCreated(SurfaceControl.Transaction t) {
        setWindowStopped(false);
    }
    @Override
    public void surfaceDestroyed() {
        setWindowStopped(true);
        mRemoteAccessibilityController.disassosciateHierarchy();
    }
    @Override
    public void surfaceReplaced(Transaction t) {
        if (mSurfaceControl != null && mBackgroundControl != null) {
            updateRelativeZ(t);
        }
    }
    private void updateRelativeZ(Transaction t) {
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot == null) {
            // We were just detached.
            return;
        }
        final SurfaceControl viewRootControl = viewRoot.getSurfaceControl();
        t.setRelativeLayer(mBackgroundControl, viewRootControl, Integer.MIN_VALUE);
        t.setRelativeLayer(mSurfaceControl, viewRootControl, mSubLayer);
    }
    public void setChildSurfacePackage(SurfaceControlViewHost.SurfacePackage p) {
        final SurfaceControl lastSc = mSurfacePackage != null ?
                mSurfacePackage.getSurfaceControl() : null;
        final SurfaceControl.Transaction transaction = new Transaction();
        if (mSurfaceControl != null) {
            if (lastSc != null) {
                transaction.reparent(lastSc, null);
                mSurfacePackage.release();
            }
            reparentSurfacePackage(transaction, p);
            applyTransactionOnVriDraw(transaction);
        }
        mSurfacePackage = p;
        invalidate();
    }
    private void reparentSurfacePackage(SurfaceControl.Transaction t,
            SurfaceControlViewHost.SurfacePackage p) {
        final SurfaceControl sc = p.getSurfaceControl();
        if (sc == null || !sc.isValid()) {
            return;
        }
        initEmbeddedHierarchyForAccessibility(p);
        t.reparent(sc, mBlastSurfaceControl).show(sc);
    }
    @Override
    public void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfoInternal(info);
        if (!mRemoteAccessibilityController.connected()) {
            return;
        }
        // Add a leashed child when this AdaptiveLayin embeds another view hierarchy. Getting this
        // leashed child would return the root node in the embedded hierarchy
        info.addChild(mRemoteAccessibilityController.getLeashToken());
    }
    @Override
    public int getImportantForAccessibility() {
        final int mode = super.getImportantForAccessibility();
        if ((mRemoteAccessibilityController!= null && !mRemoteAccessibilityController.connected())
                || mode != IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
            return mode;
        }
        return IMPORTANT_FOR_ACCESSIBILITY_YES;
    }
    private void initEmbeddedHierarchyForAccessibility(SurfaceControlViewHost.SurfacePackage p) {
        final IAccessibilityEmbeddedConnection connection = p.getAccessibilityEmbeddedConnection();
        if (mRemoteAccessibilityController.alreadyAssociated(connection)) {
            return;
        }
        mRemoteAccessibilityController.assosciateHierarchy(connection,
            getViewRootImpl().mLeashToken, getAccessibilityViewId());
        updateEmbeddedAccessibilityMatrix(true);
    }
    private void notifySurfaceDestroyed() {
        if (mSurface.isValid()) {
            SurfaceHolder.Callback[] callbacks = getSurfaceCallbacks();
            for (SurfaceHolder.Callback c : callbacks) {
                c.surfaceDestroyed(mSurfaceHolder);
            }
            if (mSurface.isValid()) {
                mSurface.forceScopedDisconnect();
            }
        }
    }
    void updateEmbeddedAccessibilityMatrix(boolean force) {
        if (!mRemoteAccessibilityController.connected()) {
            return;
        }
        getBoundsOnScreen(mTmpRect);
        mTmpRect.offset(-mAttachInfo.mWindowLeft, -mAttachInfo.mWindowTop);
        mTmpMatrix.reset();
        mTmpMatrix.setTranslate(mTmpRect.left, mTmpRect.top);
        mTmpMatrix.postScale(mScreenRect.width() / (float) mSurfaceWidth,
                mScreenRect.height() / (float) mSurfaceHeight);
        mRemoteAccessibilityController.setWindowMatrix(mTmpMatrix, force);
    }
    @Override
    protected void onFocusChanged(boolean gainFocus, @FocusDirection int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (mSurfacePackage == null || viewRoot == null) {
            return;
        }
        try {
            viewRoot.mWindowSession.grantEmbeddedWindowFocus(viewRoot.mWindow, mSurfacePackage.getInputToken(), gainFocus);
        } catch (Exception e) {
            Log.e(TAG, System.identityHashCode(this) + "Exception requesting focus on embedded window", e);
        }
    }
    private void applyTransactionOnVriDraw(Transaction t) {
        final ViewRootImpl viewRoot = getViewRootImpl();
        if (viewRoot != null) {
            // If we are using BLAST, merge the transaction with the viewroot buffer transaction.
            viewRoot.applyTransactionOnDraw(t);
        } else {
            t.apply();
        }
    }
    public void syncNextFrame(Consumer<Transaction> t) {
        mBlastBufferQueue.syncNextTransaction(t);
    }
}


