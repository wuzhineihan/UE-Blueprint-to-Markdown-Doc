// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * BP2AI 导出配置常量
 * 集中管理所有导出相关的配置参数
 * 
 * 修改这些值后需要重新编译项目
 */
namespace BP2AIExportConfig
{
	/**
	 * ========================================
	 * 日志预览配置 (Preview Settings)
	 * ========================================
	 */

	/**
	 * bp2ai.Preview.Enabled
	 * 作用：开关 ExecFlow 日志预览（LogExportResult 中的多块日志）
	 * 使用场景：批量导出时关闭，减少日志噪音；调试单个蓝图时打开
	 */
	constexpr bool bPreviewEnabled = true;

	/**
	 * bp2ai.Preview.MaxBlocks
	 * 作用：限制每个 Graph 的预览区块数量（例如只看前 3 个事件）
	 * 使用场景：防止事件多时刷屏；验证是否丢块时可临时调大
	 * 建议值：1-10
	 */
	constexpr int32 PreviewMaxBlocks = 3;

	/**
	 * bp2ai.Preview.LinesPerBlock
	 * 作用：每个预览区块内展示的行数（摘要深度）
	 * 使用场景：快速概览调整；性能与信息密度折中
	 * 建议值：3-20
	 */
	constexpr int32 PreviewLinesPerBlock = 6;

	/**
	 * bp2ai.Preview.MaxLineLength
	 * 作用：单行截断长度，防止巨型参数串污染日志
	 * 使用场景：遇到长函数调用/复杂结构体参数时保证可读性
	 * 建议值：80-300
	 */
	constexpr int32 PreviewMaxLineLength = 200;

	/**
	 * ========================================
	 * 导出格式配置 (Export Format Settings)
	 * ========================================
	 */

	/**
	 * 是否显示函数调用的默认参数
	 * true: Print(InString="Hello", bPrintToScreen=true, bPrintToLog=true, ...)
	 * false: Print(InString="Hello")
	 */
	constexpr bool bShowDefaultParams = false;

	/**
	 * 是否将用户定义的函数图表与事件图表分开显示
	 * true: 函数独立成章节
	 * false: 所有图表按类型分组
	 */
	constexpr bool bSeparateUserGraphs = true;
}

