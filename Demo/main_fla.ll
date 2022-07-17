; ModuleID = 'main.ll'
source_filename = "main.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [16 x i8] c"fla split_num=7\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [9 x i8] c"main.cpp\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [8 x i8] c"Hello~\0A\00", align 1
@.str.3 = private unnamed_addr constant [11 x i8] c"ret is %d\0A\00", align 1
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32, i8* }] [{ i8*, i8*, i8*, i32, i8* } { i8* bitcast (i32 (i32, i32)* @_Z3addii to i8*), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 5, i8* null }], section "llvm.metadata"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3addii(i32 noundef %0, i32 noundef %1) #0 {
  %.reg2mem6 = alloca i32, align 4
  %.reg2mem4 = alloca i32, align 4
  %.reg2mem2 = alloca i32, align 4
  %.reg2mem = alloca i32*, align 8
  %3 = alloca i32, align 4
  %swVar.ptr = alloca i32, align 4
  store i32 2075477681, i32* %swVar.ptr, align 4
  br label %dispatchBB

dispatchBB:                                       ; preds = %2, %returnBB
  %swVar = load i32, i32* %swVar.ptr, align 4
  switch i32 %swVar, label %swDefault [
    i32 2075477681, label %.split
    i32 843495324, label %.split.split
    i32 1901855798, label %.split.split.split
    i32 575426227, label %.split.split.split.split
    i32 11358184, label %.split.split.split.split.split
    i32 171394088, label %.split.split.split.split.split.split
    i32 698853322, label %.split.split.split.split.split.split.split
  ]

swDefault:                                        ; preds = %dispatchBB
  br label %returnBB

.split:                                           ; preds = %dispatchBB
  %4 = alloca i32, align 4
  store i32* %4, i32** %.reg2mem, align 8
  store i32 843495324, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split:                                     ; preds = %dispatchBB
  store i32 %0, i32* %3, align 4
  store i32 1901855798, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split.split:                               ; preds = %dispatchBB
  %.reload1 = load volatile i32*, i32** %.reg2mem, align 8
  store i32 %1, i32* %.reload1, align 4
  store i32 575426227, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split.split.split:                         ; preds = %dispatchBB
  %5 = load i32, i32* %3, align 4
  store i32 %5, i32* %.reg2mem2, align 4
  store i32 11358184, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split.split.split.split:                   ; preds = %dispatchBB
  %.reload = load volatile i32*, i32** %.reg2mem, align 8
  %6 = load i32, i32* %.reload, align 4
  store i32 %6, i32* %.reg2mem4, align 4
  store i32 171394088, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split.split.split.split.split:             ; preds = %dispatchBB
  %.reload3 = load volatile i32, i32* %.reg2mem2, align 4
  %.reload5 = load volatile i32, i32* %.reg2mem4, align 4
  %7 = add nsw i32 %.reload3, %.reload5
  store i32 %7, i32* %.reg2mem6, align 4
  store i32 698853322, i32* %swVar.ptr, align 4
  br label %returnBB

.split.split.split.split.split.split.split:       ; preds = %dispatchBB
  %.reload7 = load volatile i32, i32* %.reg2mem6, align 4
  ret i32 %.reload7

returnBB:                                         ; preds = %.split.split.split.split.split.split, %.split.split.split.split.split, %.split.split.split.split, %.split.split.split, %.split.split, %.split, %swDefault
  br label %dispatchBB
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z9say_hellov() #1 {
  %1 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i64 0, i64 0))
  ret void
}

declare dso_local i32 @printf(i8* noundef, ...) #2

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #3 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @_Z9say_hellov()
  %3 = call noundef i32 @_Z3addii(i32 noundef 10, i32 noundef 20)
  store i32 %3, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([11 x i8], [11 x i8]* @.str.3, i64 0, i64 0), i32 noundef %4)
  ret i32 0
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.1"}
